#include "mfem.hpp"
#include "helper.hpp"

using namespace mfem;
using namespace std;


class ParabolicEquation : public TimeDependentOperator
{
   // member variables
private:
protected:
   BilinearForm &K, &M;
   std::unique_ptr<SparseMatrix> mat;
   LinearForm &load;
   mutable std::unique_ptr<Solver> solver;
   mutable Vector y_tmp;
   bool isOperatorTimeDependent;
   real_t prev_dt = mfem::infinity();
public:
   // member functions
private:
protected:
public:
   ParabolicEquation(BilinearForm &K, BilinearForm &M, LinearForm &load,
                     bool isOperatorTimeDependent=false)
      :TimeDependentOperator(K.FESpace()->GetTrueVSize()), K(K), M(M), load(load),
       isOperatorTimeDependent(isOperatorTimeDependent)
   { }

   void SetTime(real_t t) override
   {
      if (isOperatorTimeDependent)
      {
         K.Assemble(); M.Assemble();
      }
      load.Assemble();
   }


   // Solve M^{-1}(Kx - load) = y
   void Mult(const Vector &x, Vector &y) const override
   {
      if (!solver)
      {
         CGSolver * cg = new CGSolver;
         cg->SetRelTol(1e-10);
         cg->SetAbsTol(1e-10);
         cg->SetMaxIter(1e06);
         cg->SetPrintLevel(0);
         solver.reset(cg);
      }
      if (isOperatorTimeDependent)
      {
         K.Assemble(); M.Assemble();
         K.Finalize(); M.Finalize();
      }
      load.Assemble();
      y_tmp.SetSize(x.Size());
      y_tmp = load;
      K.AddMult(x, y_tmp, -1.0);
      solver->SetOperator(M.SpMat());
      y = y_tmp;
      solver->Mult(y_tmp, y);
   }

   void ImplicitSolve(const real_t dt, const Vector &x, Vector &y) override
   {
      if (!solver)
      {
         CGSolver * cg = new CGSolver;
         cg->SetRelTol(1e-10);
         cg->SetAbsTol(1e-10);
         cg->SetMaxIter(1e06);
         cg->SetPrintLevel(0);
         solver.reset(cg);
      }
      if (isOperatorTimeDependent)
      {
         K.Assemble(); M.Assemble();
         K.Finalize(); M.Finalize();
      }
      if (dt != prev_dt || isOperatorTimeDependent)
      {
         mat.reset(Add(dt, K.SpMat(), 1.0, M.SpMat()));
         solver->SetOperator(*mat);
      }

      y_tmp = load;
      K.AddMult(x, y_tmp, -1.0);

      solver->Mult(y_tmp, y);

      prev_dt = dt;
   }
};

int main(int argc, char *argv[])
{
   int order = 1;
   int ref_levels = 1;
   int ode_solver_type = 21;
   bool vis = false;
   real_t dt = 0.00001;

   OptionsParser args(argc, argv);
   args.AddOption(&order, "-o", "--order",
                  "Polynomial order for the finite element space.");
   args.AddOption(&ref_levels, "-r", "--refine",
                  "Number of uniform refinements.");
   args.AddOption(&dt, "-dt", "--time-step",
                  "Time step size.");
   args.AddOption(&vis, "-v", "--visualize", "--no-vis", "--no-visualization",
                  "-v 1 to visualize the solution.");
   args.AddOption(&ode_solver_type, "-s", "--ode-solver",
                  ODESolver::Types.c_str());
   args.ParseCheck();


   Mesh mesh = Mesh::MakeCartesian2D(2, 2, Element::Type::QUADRILATERAL);
   for (int i=0; i<ref_levels; i++)
   {
      mesh.UniformRefinement();
   }

   const int dim = mesh.SpaceDimension();

   real_t t = 0.0;
   const real_t t_final = 1.0;

   H1_FECollection fec(order, dim);
   FiniteElementSpace fes(&mesh, &fec);

   constexpr real_t pi = M_PI;
   FunctionCoefficient load_cf([pi, &t](const Vector &x)
   {
      return (2*sin(5*t)*std::pow(pi,2.0) + 5*cos(5*t))*cos(pi*x[0])*cos(pi*x[1]);
   });
   FunctionCoefficient u0_cf([pi, &t](const Vector &x)
   {
      return sin(5*t)*cos(pi*x[0])*cos(pi*x[1]);
   });


   GridFunction u(&fes);
   u.ProjectCoefficient(u0_cf);

   LinearForm load(&fes);
   load.AddDomainIntegrator(new DomainLFIntegrator(load_cf));
   load.Assemble();

   BilinearForm diffusion(&fes);
   diffusion.AddDomainIntegrator(new DiffusionIntegrator());
   diffusion.Assemble();
   diffusion.Finalize();
   BilinearForm mass(&fes);
   mass.AddDomainIntegrator(new MassIntegrator());
   mass.Assemble();
   mass.Finalize();

   ParabolicEquation parabolic(diffusion, mass, load);
   parabolic.SetTime(t);

   unique_ptr<ODESolver> ode_solver = ODESolver::Select(ode_solver_type);
   ode_solver->Init(parabolic);

   GLVis glvis("localhost", 19916, false);
   glvis.Append(u, "u");
   glvis.Update();
   glvis.SetBound(0, -1.0, 1.0);
   real_t visualization_dt = 0.1;
   real_t last_visualization_time = 0.0;
   while (t < t_final)
   {
      real_t dt_real = min(dt, t_final - t);
      ode_solver->Step(u, t, dt_real);
      out << "time = " << t << std::endl;
      // if (t - last_visualization_time > visualization_dt)
      // {
         glvis.Update();
      //    last_visualization_time += visualization_dt;
      // }
   }
   out << "L2 error: " << u.ComputeL2Error(u0_cf) << std::endl;
}
