// Distributed under the MIT License.
// See LICENSE.txt for details.

#pragma once

#include <cstddef>
#include <limits>

template <size_t>
class Index;

/*!
 * \ingroup DataStructuresGroup
 * \brief Iterate over a (dim-1)-dimensional slice
 */
class SliceIterator {
 public:
  /*!
   * @param extents the number of grid points in each dimension
   * @param fixed_dim the dimension to slice in
   * @param fixed_index the index of the `fixed_dim` to slice at
   */
  template <size_t Dim>
  SliceIterator(const Index<Dim>& extents, size_t fixed_dim,
                size_t fixed_index);

  /// Returns `true` if the iterator is valid
  explicit operator bool() const noexcept { return volume_offset_ < size_; }

  /// Step to the next grid point
  SliceIterator& operator++();

  /// Offset into a Dim-dimensional DataVector at the current gridpoint.
  /// Note that the size of the DataVector is assumed to be the product of the
  /// extents used to construct this SliceIterator
  size_t volume_offset() const noexcept { return volume_offset_; }

  /// Offset into a (Dim-1)-dimensional DataVector at the current gridpoint.
  /// Note that the size of the DataVector is assumed to be the product of the
  /// extents used to construct this SliceIterator divided by the extent in
  /// the fixed_dim used to construct this SliceIterator
  size_t slice_offset() const noexcept { return slice_offset_; }

  /// Reset the iterator
  void reset();

 private:
  size_t size_ = std::numeric_limits<size_t>::max();
  size_t stride_ = std::numeric_limits<size_t>::max();
  size_t stride_count_ = std::numeric_limits<size_t>::max();
  size_t jump_ = std::numeric_limits<size_t>::max();
  size_t initial_offset_ = std::numeric_limits<size_t>::max();
  size_t volume_offset_ = std::numeric_limits<size_t>::max();
  size_t slice_offset_ = std::numeric_limits<size_t>::max();
};
