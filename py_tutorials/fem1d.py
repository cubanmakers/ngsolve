from netgen.meshing import *
from netgen.csg import *


# generate a 1D mesh

m = Mesh()
m.dim = 1

nel = 10

pnums = []
for i in range(0, nel+1):
    pnums.append (m.Add (MeshPoint (Pnt(i/nel, 0, 0))))

for i in range(0,nel):
    m.Add (Element1D ([pnums[i],pnums[i+1]], index=1))
# m.Save("test.vol")




from ngsolve import *
ngsmesh = Mesh(m)

V = H1(ngsmesh, order=2)
V.FreeDofs().Clear(0)    # left boundary
V.FreeDofs().Clear(nel)  # right boundary
print (V.FreeDofs())


a = BilinearForm(V)
a += Laplace(1)
a.Assemble()

print (a.mat)

f = LinearForm(V)    
f += Source(1)
f.Assemble()

u = GridFunction(V)
u.vec.data = a.mat.Inverse(V.FreeDofs()) * f.vec

print ("sol =", u.vec)

pnts = []
for i in range(101): pnts.append (i/100)

pnts_vals = [ (x,u(x)) for x in pnts if ngsmesh.Contains(x)]
# print (pnts_vals)


import matplotlib.pyplot as plt
pnts,vals = zip(*pnts_vals)
plt.plot(pnts,vals, "-*")

# plt.ion()
# plt.show()

