#ifndef BVH_BVH_HPP
#define BVH_BVH_HPP

#include <climits>
#include <memory>
#include <cassert>

#include "bvh/bounding_box.hpp"
#include "bvh/utilities.hpp"

namespace bvh {

	/// This structure represents a BVH with a list of nodes and primitives indices.
	/// The memory layout is such that the children of a node are always grouped together.
	/// This means that each node only needs one index to point to its children, as the other
	/// child can be obtained by adding one to the index of the first child. The root of the
	/// hierarchy is located at index 0 in the array of nodes.
	template <typename Scalar>
	struct Bvh {
		using IndexType = typename SizedIntegerType<sizeof(Scalar) * CHAR_BIT>::Unsigned;
		using ScalarType = Scalar;

		// Custom node - cylinder.
		struct CustomNode {
			Vector3<Scalar> p1, axis;
			Scalar h, r;
			bool is_leaf : 1;
			IndexType primitive_count : sizeof(IndexType)* CHAR_BIT - 1;
			IndexType first_child_or_primitive;

			struct BoundingBoxProxy {
				CustomNode& node;

				BoundingBoxProxy(CustomNode& node) : node(node)
				{}

				BoundingBoxProxy& operator = (const BoundingCyl<Scalar>& cyl) {
					node.p1 = cyl.c;
					node.axis = cyl.axis;
					node.r = cyl.r;
					node.h = cyl.h;
					return *this;
				}

				operator BoundingCyl<Scalar>() const {
					return BoundingCyl<Scalar>(node.p1, node.axis, node.h, node.r);
				}

				BoundingCyl<Scalar> to_bounding_box() const {
					return static_cast<BoundingCyl<Scalar>>(*this);
				}

				Scalar half_area() const { return to_bounding_box().half_area(); }

				BoundingBoxProxy& extend(const BoundingBox<Scalar>& bbox) {
					return *this = to_bounding_box().extend(bbox);
				}

			};

			BoundingBoxProxy bounding_box_proxy() {
				return BoundingBoxProxy(*this);
			}

			const BoundingBoxProxy bounding_box_proxy() const {
				return BoundingBoxProxy(*const_cast<CustomNode*>(this));
			}
		};

		// The size of this structure should be 32 bytes in
		// single precision and 64 bytes in double precision.
		struct Node {
			Scalar bounds[6];
			bool is_leaf : 1;
			IndexType primitive_count : sizeof(IndexType)* CHAR_BIT - 1;
			IndexType first_child_or_primitive;
			IndexType origin = 0;

			/// Accessor to simplify the manipulation of the bounding box of a node.
			/// This type is convertible to a `BoundingBox`.
			struct BoundingBoxProxy {
				Node& node;

				BoundingBoxProxy(Node& node)
					: node(node)
				{}

				BoundingBoxProxy& operator = (const BoundingBox<Scalar>& bbox) {
					node.bounds[0] = bbox.min[0];
					node.bounds[1] = bbox.max[0];
					node.bounds[2] = bbox.min[1];
					node.bounds[3] = bbox.max[1];
					node.bounds[4] = bbox.min[2];
					node.bounds[5] = bbox.max[2];
					return *this;
				}

				operator BoundingBox<Scalar>() const {
					return BoundingBox<Scalar>(
						Vector3<Scalar>(node.bounds[0], node.bounds[2], node.bounds[4]),
						Vector3<Scalar>(node.bounds[1], node.bounds[3], node.bounds[5]));
				}

				BoundingBox<Scalar> to_bounding_box() const {
					return static_cast<BoundingBox<Scalar>>(*this);
				}

				Scalar half_area() const { return to_bounding_box().half_area(); }

				BoundingBoxProxy& extend(const BoundingBox<Scalar>& bbox) {
					return *this = to_bounding_box().extend(bbox);
				}

				BoundingBoxProxy& extend(const Vector3<Scalar>& vector) {
					return *this = to_bounding_box().extend(vector);
				}
			};

			BoundingBoxProxy bounding_box_proxy() {
				return BoundingBoxProxy(*this);
			}

			const BoundingBoxProxy bounding_box_proxy() const {
				return BoundingBoxProxy(*const_cast<Node*>(this));
			}
		};

		/// Given a node index, returns the index of its sibling.
		static size_t sibling(size_t index) {
			assert(index != 0);
			return index % 2 == 1 ? index + 1 : index - 1;
		}

		/// Returns true if the given node is the left sibling of another.
		static bool is_left_sibling(size_t index) {
			assert(index != 0);
			return index % 2 == 1;
		}

		std::unique_ptr<Node[]>   nodes;
		std::unique_ptr<CustomNode[]>   cnodes;
		std::unique_ptr<size_t[]> primitive_indices;
		bool cylinder = false;
		bool hybrid = false;
		size_t node_count = 0;
	};

} // namespace bvh

#endif
