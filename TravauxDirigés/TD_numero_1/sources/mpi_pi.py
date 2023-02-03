from mpi4py import MPI
import numpy
import time
import numpy as np
from pprint import pprint

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

N = 400_000_000
my_N = N // size
beg = None

if rank == 0:
    print()
    beg = time.time()
    print ('We will be scattering:', N, 'in batch of', my_N)
   
comm.scatter(None, root=0)

print("heleau")
x = 2.*np.random.random_sample((my_N,))-1.
y = 2.*np.random.random_sample((my_N,))-1.
print("helppp")

filtre = np.array(x*x+y*y<1.)

S = np.add.reduce(filtre, 0)

S = comm.reduce(S,root=0)

if rank == 0:
    approx_pi = 4.*S/N
    end = time.time()

    print(f"Temps pour calculer pi : {end - beg} secondes")
    print(f"Pi vaut environ {approx_pi}")