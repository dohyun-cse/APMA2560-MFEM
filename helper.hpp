#ifndef MFEM_HELPER_HPP
#define MFEM_HELPER_HPP

#include <iomanip>
#include <iostream>
#include <vector>
#include "mfem.hpp"

namespace mfem
{

class GLVis
{
   Array<mfem::socketstream *> sockets;
   Array<mfem::GridFunction *> gfs;
   Array<Mesh *> meshes;
   bool parallel;
   const char *hostname;
   const int port;
   bool secure;

public:
#ifdef MFEM_USE_GNUTLS
   static const bool secure_default = true;
#else
   static const bool secure_default = false;
#endif
   GLVis(const char hostname[], int port, bool parallel,
         bool secure = secure_default)
      : sockets(0), gfs(0), meshes(0), parallel(parallel), hostname(hostname),
        port(port), secure(secure_default) {}

   ~GLVis()
   {
      for (socketstream *socket : sockets)
      {
         if (socket)
         {
            delete socket;
         }
      }
   }

   void Append(GridFunction &gf, const char window_title[] = nullptr,
               const char keys[] = nullptr)
   {
      sockets.Append(new socketstream(hostname, port, secure));
      socketstream *socket = sockets.Last();
      if (!socket->is_open())
      {
         return;
      }
      Mesh *mesh = gf.FESpace()->GetMesh();
      gfs.Append(&gf);
      meshes.Append(mesh);
      socket->precision(8);
#ifdef MFEM_USE_MPI
      if (parallel)
      {
         *socket << "parallel " << Mpi::WorldSize() << " " << Mpi::WorldRank() << "\n";
      }
#endif
      *socket << "solution\n" << *mesh << gf;
      if (keys)
      {
         *socket << "keys " << keys << "\n";
      }
      if (window_title)
      {
         *socket << "window_title '" << window_title <<"'\n";
      }
      *socket << std::flush;
   }

   void Update()
   {
      for (int i=0; i<sockets.Size(); i++)
      {
         if (!sockets[i]->is_open() && !sockets[i]->good())
         {
            continue;
         }
#ifdef MFEM_USE_MPI
         if (parallel)
         {
            *sockets[i] << "parallel " << Mpi::WorldSize() << " " << Mpi::WorldRank() <<
                           "\n";
         }
#endif
         *sockets[i] << "solution\n" << *meshes[i] << *gfs[i];
         *sockets[i] << std::flush;
      }
   }

   void Pause()
   {
      for (int i = 0; i < sockets.Size(); i++)
      {
         if (!sockets[i]->is_open() && !sockets[i]->good())
         {
            continue;
         }
         *sockets[i] << "pause\n" << std::flush;
      }
   }

   void SetBound(const int i, const real_t lower, const real_t upper)
   {
      if (!sockets[i]->is_open() && !sockets[i]->good())
      {
         MFEM_ABORT("Socket is not open");
      }
      *sockets[i] << "autoscale off\n valuerange " << lower << " " << upper << "\n" <<
                  std::flush;
   }


   socketstream &GetSocket(int i)
   {
      return *sockets[i];
   }
};
} // namespace mfem
#endif
