include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

if (USE_GUI)
    add_definitions(-DTCL)
endif (USE_GUI)

if(NOT WIN32)
    set(ngcomp_object_libs $<TARGET_OBJECTS:ngmg> )
endif(NOT WIN32)

add_library( ngcomp ${NGS_LIB_TYPE}
        ${ngcomp_object_libs}
        bilinearform.cpp facetfespace.cpp fespace.cpp 
        gridfunction.cpp h1hofespace.cpp hcurlhdivfes.cpp hcurlhofespace.cpp 
        hdivfes.cpp hdivhofespace.cpp hierarchicalee.cpp l2hofespace.cpp     
        linearform.cpp meshaccess.cpp ngsobject.cpp postproc.cpp	     
        preconditioner.cpp vectorfacetfespace.cpp numberfespace.cpp bddc.cpp 
        hypre_precond.cpp 
        python_comp.cpp basenumproc.cpp pde.cpp pdeparser.cpp vtkoutput.cpp
        )

if(NOT WIN32)
    target_link_libraries (ngcomp interface ngfem ngla ngbla ngstd ${TCL_LIBRARY} ${MPI_CXX_LIBRARIES} ${PYTHON_LIBS} ${LAPACK_LIBRARIES})
    install( TARGETS ngcomp ${ngs_install_dir} )

    if(NOT USE_NATIVE_ARCH)
        add_library(ngcomp_avx2 ${NGS_LIB_TYPE} $<TARGET_PROPERTY:ngcomp,SOURCES> )
        target_link_libraries(ngcomp_avx2 $<TARGET_PROPERTY:ngcomp,INTERFACE_LINK_LIBRARIES> )
        target_compile_options(ngcomp_avx2 PUBLIC ${AVX2_FLAG})
        install( TARGETS ngcomp_avx2 ${ngs_install_dir} )
    endif(NOT USE_NATIVE_ARCH)

endif(NOT WIN32)

install( FILES
        bilinearform.hpp comp.hpp facetfespace.hpp	   
        fespace.hpp gridfunction.hpp h1hofespace.hpp hcurlhdivfes.hpp	   
        hcurlhofespace.hpp hdivfes.hpp hdivhofespace.hpp		   
        l2hofespace.hpp linearform.hpp meshaccess.hpp ngsobject.hpp	   
        postproc.hpp preconditioner.hpp vectorfacetfespace.hpp hypre_precond.hpp 
        pde.hpp numproc.hpp vtkoutput.hpp
        DESTINATION include
        COMPONENT ngsolve_devel
       )

