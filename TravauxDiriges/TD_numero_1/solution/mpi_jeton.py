from mpi4py import MPI
import numpy

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
N = 8

print()


if rank == 0:
    i = 1
    comm.send(i, dest=1)

    i = comm.recv(source=N-1)
    print(i)
elif 1 <= rank:
    i = comm.recv(source=rank - 1) + 1
    comm.send(i, dest=(rank + 1) % N)