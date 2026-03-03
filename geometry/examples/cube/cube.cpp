#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <GC_MakeSegment.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Writer.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <iostream>

int main() {
  // Define points
  gp_Pnt point1(0.0, 0.0, 0.0);
  gp_Pnt point2(1.0, 0.0, 0.0);
  gp_Pnt point3(1.0, 1.0, 0.0);
  gp_Pnt point4(0.0, 1.0, 0.0);

  // Make segments
  Handle(Geom_TrimmedCurve) segment1 = GC_MakeSegment(point1, point2);
  Handle(Geom_TrimmedCurve) segment2 = GC_MakeSegment(point2, point3);
  Handle(Geom_TrimmedCurve) segment3 = GC_MakeSegment(point3, point4);
  Handle(Geom_TrimmedCurve) segment4 = GC_MakeSegment(point4, point1);

  // Make edges
  TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(segment1);
  TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(segment2);
  TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(segment3);
  TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(segment4);

  // Create wire
  TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge1, edge2, edge3, edge4);

  // Create face
  TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);

  // Extrude square
  gp_Vec vec(0, 0, 1.0);
  TopoDS_Shape body = BRepPrimAPI_MakePrism(face, vec);

  // Write STEP
  std::string step_file("cube.step");
  STEPControl_Writer step_writer;
  step_writer.Transfer(body, STEPControl_ManifoldSolidBrep);
  step_writer.Write(step_file.c_str());
  std::cout << "Cube STEP file written to: " << step_file << std::endl;

  // Create mesh
  BRepMesh_IncrementalMesh mesh(body, 0.01);

  // Write STL
  std::string stl_file("cube.stl");
  StlAPI_Writer stl_writer;
  stl_writer.ASCIIMode() = Standard_False;
  stl_writer.Write(body, stl_file.c_str());
  std::cout << "Cube STL file written to: " << stl_file << std::endl;

  return 0;
}
