from mpi4py import MPI

comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

if rank == 0:
   data = [(x+10)**x for x in range(size)]
   print ('We will be scattering:',data)
else:
   data = None
   
data = comm.scatter(data, root=0)
data += 1

print ('Rank',rank,'has data:',data)

newData = comm.gather(data,root=0)

if rank == 0:
   print ('Master:',newData)