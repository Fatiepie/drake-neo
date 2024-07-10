#include "drake/multibody/plant/hydroelastic_contact_info.h"

#include <gtest/gtest.h>

#include "drake/common/test_utilities/eigen_matrix_compare.h"

namespace drake {
namespace multibody {
namespace internal {
namespace {

using geometry::ContactSurface;
using geometry::GeometryId;
using geometry::MeshFieldLinear;
using geometry::SurfaceTriangle;
using geometry::TriangleSurfaceMesh;

// Creates an arbitrary surface mesh.
std::unique_ptr<TriangleSurfaceMesh<double>> MakeTriangleMesh() {
  std::vector<Vector3<double>> vertices = {
      {0.5, 0.5, -0.5},
      {-0.5, -0.5, -0.5},
      {-0.5, 0.5, -0.5},
  };
  std::vector<SurfaceTriangle> faces;
  faces.emplace_back(0, 1, 2);
  return std::make_unique<TriangleSurfaceMesh<double>>(std::move(faces),
                                                       std::move(vertices));
}

// Creates an arbitrary contact surface between the two given geometries.
std::unique_ptr<ContactSurface<double>> MakeContactSurface() {
  GeometryId arbitrary_id = GeometryId::get_new_id();
  auto mesh = MakeTriangleMesh();
  std::vector<double> e_MN(mesh->num_vertices(), 0.0);
  TriangleSurfaceMesh<double>* mesh_pointer = mesh.get();
  return std::make_unique<ContactSurface<double>>(
      arbitrary_id, arbitrary_id, std::move(mesh),
      std::make_unique<MeshFieldLinear<double, TriangleSurfaceMesh<double>>>(
          std::move(e_MN), mesh_pointer));
}

// Returns a distinct spatial force.
SpatialForce<double> MakeSpatialForce() {
  return SpatialForce<double>(Vector3<double>(1, 2, 3),
                              Vector3<double>(4, 5, 6));
}

HydroelasticContactInfo<double> CreateContactInfo(
    std::unique_ptr<ContactSurface<double>>* contact_surface,
    std::unique_ptr<HydroelasticContactInfo<double>>* contact_info) {
  // Create an arbitrary contact surface.
  *contact_surface = MakeContactSurface();

  // Create the HydroelasticContactInfo using particular spatial force.
  return HydroelasticContactInfo<double>(contact_surface->get(),
                                         MakeSpatialForce());
}

// Verifies that the HydroelasticContactInfo structure uses the raw pointer
// and the unique pointer, as appropriate, on copy construction.
GTEST_TEST(HydroelasticContactInfo, CopyConstruction) {
  std::unique_ptr<ContactSurface<double>> contact_surface;
  std::unique_ptr<HydroelasticContactInfo<double>> contact_info;
  HydroelasticContactInfo<double> copy =
      CreateContactInfo(&contact_surface, &contact_info);

  // Verify that copy construction used the raw pointer.
  EXPECT_EQ(contact_surface.get(), &copy.contact_surface());

  // Copy it again and make sure that the surface is new.
  HydroelasticContactInfo<double> copy2 = copy;
  EXPECT_NE(contact_surface.get(), &copy2.contact_surface());

  // Verify that the spatial force was copied.
  EXPECT_EQ(copy.F_Ac_W().translational(), MakeSpatialForce().translational());
  EXPECT_EQ(copy.F_Ac_W().rotational(), MakeSpatialForce().rotational());
}

// Verifies that the HydroelasticContactInfo structure transfers ownership of
// the ContactSurface.
GTEST_TEST(HydroelasticContactInfo, MoveConstruction) {
  std::unique_ptr<ContactSurface<double>> contact_surface;
  std::unique_ptr<HydroelasticContactInfo<double>> contact_info;
  HydroelasticContactInfo<double> copy =
      CreateContactInfo(&contact_surface, &contact_info);
  HydroelasticContactInfo<double> moved_copy = std::move(copy);

  // Verify that the move construction retained the raw pointer.
  EXPECT_EQ(contact_surface.get(), &moved_copy.contact_surface());

  // Verify that the spatial force was copied.
  EXPECT_EQ(moved_copy.F_Ac_W().translational(),
            MakeSpatialForce().translational());
  EXPECT_EQ(moved_copy.F_Ac_W().rotational(), MakeSpatialForce().rotational());
}

}  // namespace
}  // namespace internal
}  // namespace multibody
}  // namespace drake