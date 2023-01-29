from mpi4py import MPI
import numpy
import time
import numpy as np
from pprint import pprint

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

N = 40000000
my_N = N // size
beg = None

if rank == 0:
    print()
    beg = time.time()
    data = 2.*np.random.random_sample((size, my_N, 2))-1.
    print ('We will be scattering:',data.shape)
else:
    data = None
   
data = comm.scatter(data, root=0)

print("[%d] %s" % (comm.rank, data.shape))

x = data[:, 0]
y = data[:, 1]
filtre = np.array(x*x+y*y<1.)

S = np.add.reduce(filtre, 0)

print("[%d] %s" % (comm.rank, S))

newData = comm.gather(S,root=0)

if rank == 0:
    approx_pi = 4.*sum(newData)/N
    end = time.time()

    print(f"Temps pour calculer pi : {end - beg} secondes")
    print(f"Pi vaut environ {approx_pi}")