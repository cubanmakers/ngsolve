#ifdef NG_PYTHON

#include <../general/ngpython.hpp>
#include <csg.hpp>


using namespace netgen;

namespace netgen
{
  extern shared_ptr<NetgenGeometry> ng_geometry;
}

inline void NOOP_Deleter(void *) { ; }

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER == 190024213
namespace boost { template<> const volatile CSGeometry* get_pointer(const volatile CSGeometry* p) { return p; } }
#endif


// a shadow solid tree using shared pointers.

class SPSolid
{
  shared_ptr<SPSolid> s1, s2;
  Solid * solid;
  int bc = -1;
  string bcname = "";
  double maxh = -1;
  string material;
  bool owner;
  double red, green, blue;
  bool transp = false;
public:
  enum optyp { TERM, SECTION, UNION, SUB };

  SPSolid (Solid * as) : solid(as), owner(true), op(TERM) { ; }
  ~SPSolid () 
  {
    ; // if (owner) delete solid;
  }  
  SPSolid (optyp aop, shared_ptr<SPSolid> as1, shared_ptr<SPSolid> as2) 
    : s1(as1), s2(as2), owner(true), op(aop) 
  { 
    if (aop == UNION)
      solid = new Solid (Solid::UNION, s1->GetSolid(), s2->GetSolid());
    else if (aop == SECTION)
      solid = new Solid (Solid::SECTION, s1->GetSolid(), s2->GetSolid());
    else if (aop == SUB)
      solid = new Solid (Solid::SUB, s1->GetSolid()); // , s2->GetSolid());
  }

  Solid * GetSolid() { return solid; }
  const Solid * GetSolid() const { return solid; }

  void GiveUpOwner() 
  { 
    owner = false; 
    if (s1) s1 -> GiveUpOwner();
    if (s2) s2 -> GiveUpOwner();
  }

  void AddSurfaces(CSGeometry & geom)
  {
    if (op == TERM)
      geom.AddSurfaces (solid->GetPrimitive());
    if (s1) s1 -> AddSurfaces (geom);
    if (s2) s2 -> AddSurfaces (geom);
  }

  void SetMaterial (string mat)  { material = mat; }

  string GetMaterial ()
  {
    if (!material.empty()) return material;
    if (s1)
      {
        string s1mat = s1->GetMaterial();
        if (!s1mat.empty()) return s1mat;
      }
    if (s2)
      {
        string s2mat = s2->GetMaterial();
        if (!s2mat.empty()) return s2mat;
      }
    return material;
  }

  void SetBC(int abc) 
  {
    if (bc == -1) 
      {
        bc = abc;
        if (s1) s1 -> SetBC(bc);
        if (s2) s2 -> SetBC(bc);
        if (op == TERM)
          {
            Primitive * prim = solid -> GetPrimitive();
            for (int i = 0; i < prim->GetNSurfaces(); i++)
              prim->GetSurface(i).SetBCProperty (abc);
            // cout << "set " << prim->GetNSurfaces() << " surfaces to bc " << bc << endl;
          }
      }
  }

  void SetBCName(string name) 
  {
    if (bcname == "") 
      {
        bcname = name;
        if (s1) s1 -> SetBCName(name);
        if (s2) s2 -> SetBCName(name);
        if (op == TERM)
          {
            Primitive * prim = solid -> GetPrimitive();
            for (int i = 0; i < prim->GetNSurfaces(); i++)
              prim->GetSurface(i).SetBCName (name);
            // cout << "set " << prim->GetNSurfaces() << " surfaces to bc " << bc << endl;
          }
      }
  }



  void SetMaxH(double amaxh) 
  {
    if (maxh == -1) 
      {
        maxh = amaxh;
        if (s1) s1 -> SetMaxH(maxh);
        if (s2) s2 -> SetMaxH(maxh);
        if (op == TERM)
          {
            Primitive * prim = solid -> GetPrimitive();
            for (int i = 0; i < prim->GetNSurfaces(); i++)
              prim->GetSurface(i).SetMaxH (maxh);
          }
      }
  }

  void SetColor(double ared, double agreen, double ablue)
  {
    red = ared;
    green = agreen;
    blue = ablue;
  }

  double GetRed() const { return red; }
  double GetGreen() const { return green; }
  double GetBlue() const { return blue; }

  void SetTransparent() { transp = true; }
  bool IsTransparent() { return transp; }

private:
  optyp op;
};

inline ostream & operator<< (ostream & ost, const SPSolid & sol)
{
  ost << *sol.GetSolid();
  return ost;
}

namespace netgen
{
  extern CSGeometry * ParseCSG (istream & istr);
}

DLL_HEADER void ExportCSG() 
{
  ModuleScope module("csg");

  bp::class_<Point<2>> ("Point2d", bp::init<double,double>()) 
    .def ("__str__", &ToString<Point<2>>)
    .def(bp::self-bp::self)
    .def(bp::self+Vec<2>())
    .def(bp::self-Vec<2>())
    ;

  bp::class_<Point<3>> ("Point3d", bp::init<double,double,double>()) 
    .def ("__str__", &ToString<Point<3>>)
    .def(bp::self-bp::self)
    .def(bp::self+Vec<3>())
    .def(bp::self-Vec<3>())
    ;

  bp::def ("Pnt", FunctionPointer
           ([](double x, double y, double z) { return Point<3>(x,y,z); }));
  bp::def ("Pnt", FunctionPointer
           ([](double x, double y) { return Point<2>(x,y); }));

  bp::class_<Vec<2>> ("Vec2d", bp::init<double,double>()) 
    .def ("__str__", &ToString<Vec<3>>)
    .def(bp::self+bp::self)
    .def(bp::self-bp::self)
    .def(-bp::self)
    .def(double()*bp::self)
    .def("Norm", &Vec<2>::Length)
    ;

  bp::class_<Vec<3>> ("Vec3d", bp::init<double,double,double>()) 
    .def ("__str__", &ToString<Vec<3>>)
    .def(bp::self+bp::self)
    .def(bp::self-bp::self)
    .def(-bp::self)
    .def(double()*bp::self)
    .def("Norm", &Vec<3>::Length)
    ;

  bp::def ("Vec", FunctionPointer
           ([] (double x, double y, double z) { return Vec<3>(x,y,z); }));
  bp::def ("Vec", FunctionPointer
           ([] (double x, double y) { return Vec<2>(x,y); }));


  bp::class_<SplineGeometry<2>> ("SplineCurve2d")
    .def ("AddPoint", FunctionPointer
          ([] (SplineGeometry<2> & self, double x, double y)
           {
             self.geompoints.Append (GeomPoint<2> (Point<2> (x,y)));
             return self.geompoints.Size()-1;
           }))
    .def ("AddSegment", FunctionPointer
          ([] (SplineGeometry<2> & self, int i1, int i2)
           {
             self.splines.Append (new LineSeg<2> (self.geompoints[i1], self.geompoints[i2]));
           }))
    .def ("AddSegment", FunctionPointer
          ([] (SplineGeometry<2> & self, int i1, int i2, int i3)
           {
             self.splines.Append (new SplineSeg3<2> (self.geompoints[i1], self.geompoints[i2], self.geompoints[i3]));
           }))
    ;

  bp::class_<SplineGeometry<3>,shared_ptr<SplineGeometry<3>>> ("SplineCurve3d")
    .def ("AddPoint", FunctionPointer
          ([] (SplineGeometry<3> & self, double x, double y, double z)
           {
             self.geompoints.Append (GeomPoint<3> (Point<3> (x,y,z)));
             return self.geompoints.Size()-1;
           }))
    .def ("AddSegment", FunctionPointer
          ([] (SplineGeometry<3> & self, int i1, int i2)
           {
             self.splines.Append (new LineSeg<3> (self.geompoints[i1], self.geompoints[i2]));
           }))
    .def ("AddSegment", FunctionPointer
          ([] (SplineGeometry<3> & self, int i1, int i2, int i3)
           {
             self.splines.Append (new SplineSeg3<3> (self.geompoints[i1], self.geompoints[i2], self.geompoints[i3]));
           }))
    ;

  bp::class_<SplineSurface, shared_ptr<SplineSurface>,boost::noncopyable> ("SplineSurface")
    .def("AddPoint", FunctionPointer
	 ([] (SplineSurface & self, double x, double y, double z, bool hpref)
	  {
	    self.AppendPoint(Point<3>(x,y,z),hpref);
	    return self.GetNP()-1;
	  }),
	 (bp::arg("self"),bp::arg("x"),bp::arg("y"),bp::arg("z"),bp::arg("hpref")=false))
    .def("AddSegment", FunctionPointer
	 ([] (SplineSurface & self, int i1, int i2, string bcname)
	  {
	    auto str = new string(bcname);
	    self.AppendSegment(new LineSeg<3>(self.GetPoint(i1),self.GetPoint(i2)),str);
	  }),
	 (bp::arg("self"),bp::arg("pnt1"),bp::arg("pnt2"),bp::arg("bcname")="default"))
    ;
  
#if (BOOST_VERSION >= 106000) && (BOOST_VERSION < 106100)
  bp::register_ptr_to_python<shared_ptr<SPSolid>>();
#endif
  bp::class_<SPSolid, shared_ptr<SPSolid>, boost::noncopyable> ("Solid", bp::no_init)
    .def ("__str__", &ToString<SPSolid>)
    .def ("__add__", FunctionPointer( [] ( shared_ptr<SPSolid> self, shared_ptr<SPSolid> other) { return make_shared<SPSolid> (SPSolid::UNION, self, other); }))
    .def ("__mul__", FunctionPointer( [] ( shared_ptr<SPSolid> self, shared_ptr<SPSolid> other) { return make_shared<SPSolid> (SPSolid::SECTION, self, other); }))
    .def ("__sub__", FunctionPointer
          ([] ( shared_ptr<SPSolid> self, shared_ptr<SPSolid> other) 
           { return make_shared<SPSolid> (SPSolid::SECTION, self, 
                                          make_shared<SPSolid> (SPSolid::SUB, other, nullptr)); }))

    .def ("bc", FunctionPointer([](shared_ptr<SPSolid> & self, int nr) -> shared_ptr<SPSolid> 
                                { self->SetBC(nr); return self; }))
    .def ("bc", FunctionPointer([](shared_ptr<SPSolid> & self, string name) -> shared_ptr<SPSolid> 
                                { self->SetBCName(name); return self; }))
    .def ("maxh", FunctionPointer([](shared_ptr<SPSolid> & self, double maxh) -> shared_ptr<SPSolid> 
                                { self->SetMaxH(maxh); return self; }))
    .def ("mat", FunctionPointer([](shared_ptr<SPSolid> & self, string mat) -> shared_ptr<SPSolid> 
                                 { self->SetMaterial(mat); return self; }))
    .def ("mat", &SPSolid::GetMaterial)
    .def("col", FunctionPointer([](shared_ptr<SPSolid> & self, bp::list rgb) -> shared_ptr<SPSolid>
                                { 
                                  bp::extract<double> red(rgb[0]);
                                  bp::extract<double> green(rgb[1]);
                                  bp::extract<double> blue(rgb[2]);
                                  self->SetColor(red(),green(),blue());
                                  return self; 
                                }))
    .def("transp", FunctionPointer([](shared_ptr<SPSolid> & self)->shared_ptr < SPSolid > { self->SetTransparent(); return self; }))
    ;

  bp::def ("Sphere", FunctionPointer([](Point<3> c, double r)
                                     {
                                       Sphere * sp = new Sphere (c, r);
                                       Solid * sol = new Solid (sp);
                                       return make_shared<SPSolid> (sol);
                                     }));
  bp::def ("Plane", FunctionPointer([](Point<3> p, Vec<3> n)
                                    {
                                      Plane * sp = new Plane (p,n);
                                      Solid * sol = new Solid (sp);
                                      return make_shared<SPSolid> (sol);
                                    }));
  bp::def ("Cone", FunctionPointer([](Point<3> a, Point<3> b, double ra, double rb)
                                       {
                                         Cone * cyl = new Cone (a, b, ra, rb);
                                         Solid * sol = new Solid (cyl);
                                         return make_shared<SPSolid> (sol);
                                       }));
  bp::def ("Cylinder", FunctionPointer([](Point<3> a, Point<3> b, double r)
                                       {
                                         Cylinder * cyl = new Cylinder (a, b, r);
                                         Solid * sol = new Solid (cyl);
                                         return make_shared<SPSolid> (sol);
                                       }));
  bp::def ("OrthoBrick", FunctionPointer([](Point<3> p1, Point<3> p2)
                                         {
                                           OrthoBrick * brick = new OrthoBrick (p1,p2);
                                           Solid * sol = new Solid (brick);
                                           return make_shared<SPSolid> (sol);
                                         }));
  bp::def ("Revolution", FunctionPointer([](Point<3> p1, Point<3> p2,
                                            const SplineGeometry<2> & spline)
                                         {
                                           Revolution * rev = new Revolution (p1, p2, spline);
                                           Solid * sol = new Solid(rev);
                                           return make_shared<SPSolid> (sol);
                                         }));
  
  bp::def ("Or", FunctionPointer([](shared_ptr<SPSolid> s1, shared_ptr<SPSolid> s2)
                                 {
                                   return make_shared<SPSolid> (SPSolid::UNION, s1, s2);
                                 }));
  bp::def ("And", FunctionPointer([](shared_ptr<SPSolid> s1, shared_ptr<SPSolid> s2)
                                  {
                                    return make_shared<SPSolid> (SPSolid::SECTION, s1, s2);
                                  }));


  bp::class_<CSGeometry,shared_ptr<CSGeometry>, boost::noncopyable> ("CSGeometry")
    .def("__init__", bp::make_constructor (FunctionPointer
                                           ([](const string & filename)
                                            {
                                              cout << "load geometry";
                                              ifstream ist(filename);
                                              shared_ptr<CSGeometry> geom(ParseCSG(ist));
                                              geom -> FindIdenticSurfaces(1e-8 * geom->MaxSize()); 
                                              return geom;
                                            })))

    .def("__init__", bp::make_constructor (FunctionPointer
                                           ([](const bp::list & solidlist)
                                            {
                                              cout << "csg from list";
                                              auto geom = make_shared<CSGeometry>();
                                              for (int i = 0; i < len(solidlist); i++)
                                                {
                                                  bp::object obj = solidlist[i];
                                                  cout << "obj " << i << endl;

                                                  bp::extract<shared_ptr<SPSolid>> solid(solidlist[i]);
                                                  if(solid.check())
                                                    {
                                                      cout << "its a solid" << endl;
                                                      solid()->AddSurfaces (*geom);
                                                      solid()->GiveUpOwner();
                                                      int tlonr = geom->SetTopLevelObject (solid()->GetSolid());
                                                      geom->GetTopLevelObject(tlonr) -> SetMaterial(solid()->GetMaterial());
                                                    }
                                                }
                                              geom -> FindIdenticSurfaces(1e-8 * geom->MaxSize()); 
                                              return geom;
                                            })))

    .def("Save", FunctionPointer([] (CSGeometry & self, string filename)
                                 {
                                   cout << "save geometry to file " << filename << endl;
                                   self.Save (filename);
                                 }))
    .def("Add", FunctionPointer
         ([] (CSGeometry & self, shared_ptr<SPSolid> solid, bp::list bcmod)
          {
            solid->AddSurfaces (self);
            solid->GiveUpOwner();
            int tlonr = self.SetTopLevelObject (solid->GetSolid());
            self.GetTopLevelObject(tlonr) -> SetMaterial(solid->GetMaterial());
            self.GetTopLevelObject(tlonr) -> SetRGB(solid->GetRed(),solid->GetGreen(),solid->GetBlue());
            self.GetTopLevelObject(tlonr)->SetTransparent(solid->IsTransparent());

            // bcmod is list of tuples ( solid, bcnr )
            for (int i = 0; i < bp::len(bcmod); i++)
              {
                bp::tuple tup = bp::extract<bp::tuple> (bcmod[i]) ();
                auto mod_solid = bp::extract<shared_ptr<SPSolid>> (tup[0]) ();
                int mod_nr = -1;
                string * bcname = nullptr;
                bp::object val = tup[1];
                if (bp::extract<int>(val).check()) mod_nr = bp::extract<int> (val)();
                if (bp::extract<string>(val).check()) bcname = new string ( bp::extract<string> (val)());

                Array<int> si;
                mod_solid -> GetSolid() -> GetSurfaceIndices (si);
                // cout << "change bc on surfaces: " << si << " to " << mod_nr << endl;

                for (int j = 0; j < si.Size(); j++)
                  {
                    CSGeometry::BCModification bcm;
                    bcm.bcname = bcname ? new string (*bcname) : nullptr;
                    bcm.tlonr = tlonr;
                    bcm.si = si[j];
		    bcm.bcnr = mod_nr;
		    self.bcmodifications.Append (bcm);
                  }
                delete bcname;
              }
            return tlonr;

          }),
         (bp::arg("self"), bp::arg("solid"), bp::arg("bcmod")=bp::list())
         )

    .def("AddSurface", FunctionPointer
         ([] (CSGeometry & self, shared_ptr<SPSolid> surface, shared_ptr<SPSolid> solid)
          {
            solid->AddSurfaces (self);
            solid->GiveUpOwner();
            Surface & surf = surface->GetSolid()->GetPrimitive()->GetSurface();
            int tlonr = self.SetTopLevelObject (solid->GetSolid(), &surf);
            // self.GetTopLevelObject(tlonr) -> SetMaterial(solid->GetMaterial());
            self.GetTopLevelObject(tlonr) -> SetBCProp(surf.GetBCProperty());
            self.GetTopLevelObject(tlonr) -> SetBCName(surf.GetBCName());
            
            self.GetTopLevelObject(tlonr) -> SetRGB(solid->GetRed(),solid->GetGreen(),solid->GetBlue());
            self.GetTopLevelObject(tlonr)->SetTransparent(solid->IsTransparent());
          }),
         (bp::arg("self"), bp::arg("surface"), bp::arg("solid"))
         )
    .def("AddSplineSurface", FunctionPointer
	 ([] (CSGeometry & self, shared_ptr<SplineSurface> surf)
	  {
	    auto planes = surf->CreatePlanes();
	    auto spsol = make_shared<SPSolid>(new Solid(&*surf));
	    for(auto plane : (*planes)){
	      spsol = make_shared<SPSolid>(SPSolid::SECTION,spsol,make_shared<SPSolid>(new Solid(plane)));
	    }
	    spsol->AddSurfaces(self);
	    int tlonr = self.SetTopLevelObject(spsol->GetSolid(), &*surf);
	  }),
	 (bp::arg("self"), bp::arg("SplineSurface")))

    .def("CloseSurfaces", FunctionPointer
         ([] (CSGeometry & self, shared_ptr<SPSolid> s1, shared_ptr<SPSolid> s2, bp::list aslices )
          {
            Array<int> si1, si2;
            s1->GetSolid()->GetSurfaceIndices (si1);
            s2->GetSolid()->GetSurfaceIndices (si2);
            cout << "surface ids1 = " << si1 << endl;
            cout << "surface ids2 = " << si2 << endl;

            Flags flags;

            try
            {
                int n = bp::len(aslices);
                Array<double> slices(n);
                for(int i=0; i<n; i++)
                {
                    slices[i]= bp::extract<double>(aslices[i])();
                }
                flags.SetFlag("slices", slices);
            }
            catch( bp::error_already_set const & ) {
                cout << "caught python error:" << endl;
                PyErr_Print();
            }

            const TopLevelObject * domain = nullptr;
            self.AddIdentification
              (new CloseSurfaceIdentification
               (self.GetNIdentifications()+1, self,
                self.GetSurface (si1[0]), self.GetSurface (si2[0]),
                domain,
                flags));
          }),
         (bp::arg("self"), bp::arg("solid1"), bp::arg("solid2"), bp::arg("slices"))
         )
    .def("CloseSurfaces", FunctionPointer
         ([] (CSGeometry & self, shared_ptr<SPSolid> s1, shared_ptr<SPSolid> s2, int reflevels)
          {
            Array<int> si1, si2;
            s1->GetSolid()->GetSurfaceIndices (si1);
            s2->GetSolid()->GetSurfaceIndices (si2);
            cout << "surface ids1 = " << si1 << endl;
            cout << "surface ids2 = " << si2 << endl;

            Flags flags;
            const TopLevelObject * domain = nullptr;
            self.AddIdentification 
              (new CloseSurfaceIdentification 
               (self.GetNIdentifications()+1, self, 
                self.GetSurface (si1[0]), self.GetSurface (si2[0]),
                domain,
                flags));
          }),
         (bp::arg("self"), bp::arg("solid1"), bp::arg("solid2"), bp::arg("reflevels")=2)
         )
    
    .def("PeriodicSurfaces", FunctionPointer
         ([] (CSGeometry & self, shared_ptr<SPSolid> s1, shared_ptr<SPSolid> s2)
          {
            Array<int> si1, si2;
            s1->GetSolid()->GetSurfaceIndices (si1);
            s2->GetSolid()->GetSurfaceIndices (si2);
            cout << "identify surfaces " << si1[0] << " and " << si2[0] << endl;
            self.AddIdentification 
              (new PeriodicIdentification 
               (self.GetNIdentifications()+1, self, 
                self.GetSurface (si1[0]), self.GetSurface (si2[0])));
          }),
         (bp::arg("self"), bp::arg("solid1"), bp::arg("solid2"))
         )
    
    .def("GetTransparent", FunctionPointer
         ([] (CSGeometry & self, int tlonr)
          {
            return self.GetTopLevelObject(tlonr)->GetTransparent();
          }),
         (bp::arg("self"), bp::arg("tlonr"))
         )
    .def("SetTransparent", FunctionPointer
         ([] (CSGeometry & self, int tlonr, bool transparent)
          {
            self.GetTopLevelObject(tlonr)->SetTransparent(transparent);
          }),
         (bp::arg("self"), bp::arg("tlonr"), bp::arg("transparent"))
         )

    .def("GetVisible", FunctionPointer
         ([] (CSGeometry & self, int tlonr)
          {
            return self.GetTopLevelObject(tlonr)->GetVisible();
          }),
         (bp::arg("self"), bp::arg("tlonr"))
         )
    .def("SetVisible", FunctionPointer
         ([] (CSGeometry & self, int tlonr, bool visible)
          {
            self.GetTopLevelObject(tlonr)->SetVisible(visible);
          }),
         (bp::arg("self"), bp::arg("tlonr"), bp::arg("visible"))
         )
    .def("SetBoundingBox", FunctionPointer
         ([] (CSGeometry & self, Point<3> pmin, Point<3> pmax)
          {
            self.SetBoundingBox(Box<3> (pmin, pmax));
          }),
         (bp::arg("self"), bp::arg("pmin"), bp::arg("pmax"))
         )
    .def("Draw", FunctionPointer
         ([] (shared_ptr<CSGeometry> self)
          {
             self->FindIdenticSurfaces(1e-6);
             self->CalcTriangleApproximation(0.01, 20);
             ng_geometry = self;
          }),
         (bp::arg("self"))
         )
    .add_property ("ntlo", &CSGeometry::GetNTopLevelObjects)
    ;

  bp::def("GenerateMesh", FunctionPointer
          ([](shared_ptr<CSGeometry> geo, MeshingParameters & param)
           {
             auto dummy = make_shared<Mesh>();
             SetGlobalMesh (dummy);
             dummy->SetGeometry(geo);
	     ng_geometry = geo;
             geo->FindIdenticSurfaces(1e-8 * geo->MaxSize());
             try
               {
                 geo->GenerateMesh (dummy, param, 0, 6);
               }
             catch (NgException ex)
               {
                 cout << "Caught NgException: " << ex.What() << endl;
               }
             return dummy;
           }))
    ;

  bp::def("Save", FunctionPointer 
          ([](const Mesh & self, const string & filename, const CSGeometry & geom)
           {
             ostream * outfile;
             if (filename.substr (filename.length()-3, 3) == ".gz")
               outfile = new ogzstream (filename.c_str());
             else
               outfile = new ofstream (filename.c_str());
             
             self.Save (*outfile);
             *outfile << endl << endl << "endmesh" << endl << endl;
             geom.SaveToMeshFile (*outfile);
             delete outfile;
           }))
    ;



  bp::def("ZRefinement", FunctionPointer
          ([](Mesh & mesh, CSGeometry & geom)
          {
            ZRefinementOptions opt;
            opt.minref = 5;
            ZRefinement (mesh, &geom, opt);
          }))
    ;
}





BOOST_PYTHON_MODULE(libcsg) {
  ExportCSG();
}


#endif

