//                                MFEM Example 1
//
// Compile with: make ex1
//
// Sample runs:  ex1 -m ../data/square-disc.mesh
//               ex1 -m ../data/star.mesh
//               ex1 -m ../data/star-mixed.mesh
//               ex1 -m ../data/escher.mesh
//               ex1 -m ../data/fichera.mesh
//               ex1 -m ../data/fichera-mixed.mesh
//               ex1 -m ../data/toroid-wedge.mesh
//               ex1 -m ../data/octahedron.mesh -o 1
//               ex1 -m ../data/periodic-annulus-sector.msh
//               ex1 -m ../data/periodic-torus-sector.msh
//               ex1 -m ../data/square-disc-p2.vtk -o 2
//               ex1 -m ../data/square-disc-p3.mesh -o 3
//               ex1 -m ../data/square-disc-nurbs.mesh -o -1
//               ex1 -m ../data/star-mixed-p2.mesh -o 2
//               ex1 -m ../data/disc-nurbs.mesh -o -1
//               ex1 -m ../data/pipe-nurbs.mesh -o -1
//               ex1 -m ../data/fichera-mixed-p2.mesh -o 2
//               ex1 -m ../data/star-surf.mesh
//               ex1 -m ../data/square-disc-surf.mesh
//               ex1 -m ../data/inline-segment.mesh
//               ex1 -m ../data/amr-quad.mesh
//               ex1 -m ../data/amr-hex.mesh
//               ex1 -m ../data/fichera-amr.mesh
//               ex1 -m ../data/mobius-strip.mesh
//               ex1 -m ../data/mobius-strip.mesh -o -1 -sc
//
// Device sample runs:
//               ex1 -pa -d cuda
//               ex1 -fa -d cuda
//               ex1 -pa -d raja-cuda
//             * ex1 -pa -d raja-hip
//               ex1 -pa -d occa-cuda
//               ex1 -pa -d raja-omp
//               ex1 -pa -d occa-omp
//               ex1 -pa -d ceed-cpu
//               ex1 -pa -d ceed-cpu -o 4 -a
//               ex1 -pa -d ceed-cpu -m ../data/square-mixed.mesh
//               ex1 -pa -d ceed-cpu -m ../data/fichera-mixed.mesh
//             * ex1 -pa -d ceed-cuda
//             * ex1 -pa -d ceed-hip
//               ex1 -pa -d ceed-cuda:/gpu/cuda/shared
//               ex1 -pa -d ceed-cuda:/gpu/cuda/shared -m ../data/square-mixed.mesh
//               ex1 -pa -d ceed-cuda:/gpu/cuda/shared -m ../data/fichera-mixed.mesh
//               ex1 -m ../data/beam-hex.mesh -pa -d cuda
//               ex1 -m ../data/beam-tet.mesh -pa -d ceed-cpu
//               ex1 -m ../data/beam-tet.mesh -pa -d ceed-cuda:/gpu/cuda/ref
//
// Description:  This example code demonstrates the use of MFEM to define a
//               simple finite element discretization of the Laplace problem
//               -Delta u = 1 with homogeneous Dirichlet boundary conditions.
//               Specifically, we discretize using a FE space of the specified
//               order, or if order < 1 using an isoparametric/isogeometric
//               space (i.e. quadratic for quadratic curvilinear mesh, NURBS for
//               NURBS mesh, etc.)
//
//               The example highlights the use of mesh refinement, finite
//               element grid functions, as well as linear and bilinear forms
//               corresponding to the left-hand side and right-hand side of the
//               discrete linear system. We also cover the explicit elimination
//               of essential boundary conditions, static condensation, and the
//               optional connection to the GLVis tool for visualization.

#include "mfem.hpp"
#include <fstream>
#include <iostream>

using namespace std;
using namespace mfem;

/// @brief Mark boundaries of a mesh based on a predicate function.
/// @param mesh The mesh whose boundaries will be marked.
/// @param attr The attribute value to assign to marked boundaries.
/// @param marker A function that takes the midpoint coordinate as input and returns true for boundaries to be marked.
/// @example MarkBoundaries(mesh, 5, [](const Vector &x) { return x[0] < 1e-9; });
/// This example marks boundaries where the x-coordinate is nearly zero, accounting for numerical errors.
void MarkBoundaries(Mesh &mesh, int attr,
                    std::function<bool(const Vector &x)> marker)
{
   const int dim = mesh.SpaceDimension();
   Vector center(mesh.SpaceDimension());
   Array<int> vertices;
   for (int i=0; i<mesh.GetNBE(); i++)
   {
      center = 0.0;
      mesh.GetBdrElement(i)->GetVertices(vertices);
      for (auto v:vertices)
      {
         real_t * coord = mesh.GetVertex(v);
         for (int d=0; d<dim; d++)
         {
            center[d] += coord[d];
         }
      }
      center *= 1.0 / vertices.Size();
      if (marker(center))
      {
         mesh.SetBdrAttribute(i, attr);
      }
   }
   mesh.SetAttributes();
}

double rhs_func(const Vector &x)
{
   return M_PI*M_PI*2.0*sin(M_PI*x[0])*sin(M_PI*x[1]);
}

double u_func(const Vector &x)
{
   return sin(M_PI*x[0])*sin(M_PI*x[1]);
}

int main(int argc, char *argv[])
{
   // 1. Parse command-line options.
   // const char *mesh_file = "../data/star.mesh";
   int order = 1;
   bool static_cond = false;
   bool pa = false;
   bool fa = false;
   const char *device_config = "cpu";
   bool visualization = true;
   bool algebraic_ceed = false;

   double eps = 1.0; // diffusion strengh
   double bx = 0.0; // convection x-direction
   double by = 0.0; // convection y-direction

   OptionsParser args(argc, argv);
   // args.AddOption(&mesh_file, "-m", "--mesh",
   //                "Mesh file to use.");
   args.AddOption(&eps, "-eps", "--diffusion",
                  "Diffusion coefficient. Default = 1");
   args.AddOption(&bx, "-bx", "--convection-x",
                  "Convection x-direction. Default = 0");
   args.AddOption(&by, "-by", "--convection-y",
                  "Convection x-direction. Default = 0");
   args.AddOption(&order, "-o", "--order",
                  "Finite element order (polynomial degree) or -1 for"
                  " isoparametric space.");
   args.AddOption(&static_cond, "-sc", "--static-condensation", "-no-sc",
                  "--no-static-condensation", "Enable static condensation.");
   args.AddOption(&pa, "-pa", "--partial-assembly", "-no-pa",
                  "--no-partial-assembly", "Enable Partial Assembly.");
   args.AddOption(&fa, "-fa", "--full-assembly", "-no-fa",
                  "--no-full-assembly", "Enable Full Assembly.");
   args.AddOption(&device_config, "-d", "--device",
                  "Device configuration string, see Device::Configure().");
#ifdef MFEM_USE_CEED
   args.AddOption(&algebraic_ceed, "-a", "--algebraic", "-no-a", "--no-algebraic",
                  "Use algebraic Ceed solver");
#endif
   args.AddOption(&visualization, "-vis", "--visualization", "-no-vis",
                  "--no-visualization",
                  "Enable or disable GLVis visualization.");
   args.Parse();
   if (!args.Good())
   {
      args.PrintUsage(cout);
      return 1;
   }
   args.PrintOptions(cout);

   // 2. Enable hardware devices such as GPUs, and programming models such as
   //    CUDA, OCCA, RAJA and OpenMP based on command line options.
   Device device(device_config);
   device.Print();

   // 3. Read the mesh from the given mesh file. We can handle triangular,
   //    quadrilateral, tetrahedral, hexahedral, surface and volume meshes with
   //    the same code.
   // Mesh mesh(mesh_file, 1, 1);
   Mesh mesh = Mesh::MakeCartesian2D(4, 4, Element::QUADRILATERAL);
   int dim = mesh.Dimension();

   // 4. Refine the mesh to increase the resolution. In this example we do
   //    'ref_levels' of uniform refinement. We choose 'ref_levels' to be the
   //    largest number that gives a final mesh with no more than 50,000
   //    elements.
   // {
   //    int ref_levels =
   //       (int)floor(log(50000./mesh.GetNE())/log(2.)/dim);
   //    for (int l = 0; l < ref_levels; l++)
   //    {
   //       mesh.UniformRefinement();
   //    }
   // }

   // 5. Define a finite element space on the mesh. Here we use continuous
   //    Lagrange finite elements of the specified order. If order < 1, we
   //    instead use an isoparametric/isogeometric space.
   FiniteElementCollection *fec;
   bool delete_fec;
   if (order > 0)
   {
      fec = new H1_FECollection(order, dim);
      delete_fec = true;
   }
   else if (mesh.GetNodes())
   {
      fec = mesh.GetNodes()->OwnFEC();
      delete_fec = false;
      cout << "Using isoparametric FEs: " << fec->Name() << endl;
   }
   else
   {
      fec = new H1_FECollection(order = 1, dim);
      delete_fec = true;
   }
   FiniteElementSpace fespace(&mesh, fec);
   cout << "Number of finite element unknowns: "
        << fespace.GetTrueVSize() << endl;

   // 6. Determine the list of true (i.e. conforming) essential boundary dofs.
   //    In this example, the boundary conditions are defined by marking all
   //    the external boundary attributes from the mesh as essential (Dirichlet)
   //    and converting them to a list of true dofs.
   Array<int> ess_bdr(mesh.bdr_attributes.Max());
   ess_bdr = 1;

   Array<int> ess_tdof_list;

   // 7. Set up the linear form b(.) which corresponds to the right-hand side of
   //    the FEM linear system, which in this case is (1,phi_i) where phi_i are
   //    the basis functions in the finite element fespace.
   LinearForm b(&fespace);
   FunctionCoefficient f(rhs_func);
   FunctionCoefficient u(u_func);
   b.AddDomainIntegrator(new DomainLFIntegrator(f));

   // 8. Define the solution vector x as a finite element grid function
   //    corresponding to fespace. Initialize x with initial guess of zero,
   //    which satisfies the boundary conditions.
   GridFunction x(&fespace);
   x = 0.0;

   // 9. Set up the bilinear form a(.,.) on the finite element space
   //    corresponding to the Laplacian operator -Delta, by adding the Diffusion
   //    domain integrator.
   BilinearForm a(&fespace);
   if (pa) { a.SetAssemblyLevel(AssemblyLevel::PARTIAL); }
   if (fa)
   {
      a.SetAssemblyLevel(AssemblyLevel::FULL);
      // Sort the matrix column indices when running on GPU or with OpenMP (i.e.
      // when Device::IsEnabled() returns true). This makes the results
      // bit-for-bit deterministic at the cost of somewhat longer run time.
      a.EnableSparseMatrixSorting(Device::IsEnabled());
   }
   ConstantCoefficient eps_cf(eps);
   a.AddDomainIntegrator(new DiffusionIntegrator(eps_cf));
   Vector beta(2);
   beta[0] = bx;
   beta[1] = by;
   VectorConstantCoefficient beta_cf(beta);
   a.AddDomainIntegrator(new ConvectionIntegrator(beta_cf));

   // 10. Assemble the bilinear form and the corresponding linear system,
   //     applying any necessary transformations such as: eliminating boundary
   //     conditions, applying conforming constraints for non-conforming AMR,
   //     static condensation, etc.
   if (static_cond) { a.EnableStaticCondensation(); }

   for (int i = 0; i < 4; i++)
   {
      mesh.UniformRefinement();
      fespace.Update();
      a.Update();
      b.Update();
      x.Update();

      a.Assemble();
      b.Assemble();
      x = 0.0;
      x.ProjectBdrCoefficient(u, ess_bdr);
      fespace.GetEssentialTrueDofs(ess_bdr, ess_tdof_list);

      OperatorPtr A;
      Vector B, X;
      a.FormLinearSystem(ess_tdof_list, x, b, A, X, B);

      cout << "Size of linear system: " << A->Height() << endl;

      // 11. Solve the linear system A X = B.
      if (!pa)
      {
#ifndef MFEM_USE_SUITESPARSE
         // Use a simple symmetric Gauss-Seidel preconditioner with PCG.
         GSSmoother M((SparseMatrix&)(*A));
         if (bx*bx + by*by == 0.0)
         {
            PCG(*A, M, B, X, 0, 2000, 1e-12, 0.0);
         }
         else
         {
            GMRES(*A, M, B, X, 0, 3000);
         }
#else
         // If MFEM was compiled with SuiteSparse, use UMFPACK to solve the system.
         UMFPackSolver umf_solver;
         umf_solver.Control[UMFPACK_ORDERING] = UMFPACK_ORDERING_METIS;
         umf_solver.SetOperator(*A);
         umf_solver.Mult(B, X);
#endif
      }
      else
      {
         if (UsesTensorBasis(fespace))
         {
            if (algebraic_ceed)
            {
               ceed::AlgebraicSolver M(a, ess_tdof_list);
               PCG(*A, M, B, X, 1, 400, 1e-12, 0.0);
            }
            else
            {
               OperatorJacobiSmoother M(a, ess_tdof_list);
               PCG(*A, M, B, X, 1, 400, 1e-12, 0.0);
            }
         }
         else
         {
            CG(*A, B, X, 1, 400, 1e-12, 0.0);
         }
      }

      // 12. Recover the solution as a finite element grid function.
      a.RecoverFEMSolution(X, b, x);

      double err = x.ComputeL2Error(u);

      out << err << std::endl;

   }

   // 14. Send the solution by socket to a GLVis server.
   if (visualization)
   {
      char vishost[] = "localhost";
      int  visport   = 19916;
      socketstream sol_sock(vishost, visport);
      sol_sock.precision(8);
      sol_sock << "solution\n" << mesh << x << flush;

      ParaViewDataCollection paraview_dc("Ex1-Modified", &mesh);
      paraview_dc.SetPrefixPath("ParaView");
      // Export high-resolution when high-order polynomials are used
      paraview_dc.SetLevelsOfDetail(order == 1 ? order : order + 3);
      paraview_dc.SetCycle(0);
      paraview_dc.SetDataFormat(VTKFormat::BINARY);
      paraview_dc.SetHighOrderOutput(true);
      paraview_dc.SetTime(0.0); // set the time
      paraview_dc.RegisterField("u",&x);
      paraview_dc.Save();

   }

   // 15. Free the used memory.
   if (delete_fec)
   {
      delete fec;
   }

   return 0;
}
