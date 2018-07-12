// Distributed under the MIT License.
// See LICENSE.txt for details.

/// \file
/// Defines class h5::Header

#pragma once

#include <hdf5.h>
#include <string>

#include "IO/H5/Object.hpp"
#include "IO/H5/OpenGroup.hpp"

namespace h5 {
/*!
 * \ingroup HDF5Group
 * \brief Writes header info about the build, git commit, branch, etc.
 *
 * A Header object is used to store the ::info_from_build() result in the HDF5
 * files. The Header is automatically added to every single file by the
 * constructor of H5File.
 *
 * \example
 * You can read the header info out of an H5 file as shown in the example:
 * \snippet Test_H5.cpp h5file_readwrite_get_header
 */
class Header : public h5::Object {
 public:
  /// \cond HIDDEN_SYMOLS
  static std::string extension() { return ".hdr"; }

  Header(bool exists, detail::OpenGroup&& group, hid_t location,
         const std::string& name);

  Header(const Header& /*rhs*/) = delete;
  Header& operator=(const Header& /*rhs*/) = delete;

  Header(Header&& /*rhs*/) noexcept = delete;             // NOLINT
  Header& operator=(Header&& /*rhs*/) noexcept = delete;  // NOLINT

  ~Header() override = default;
  /// \endcond

  const std::string& get_header() const noexcept { return header_info_; }

 private:
  /// \cond HIDDEN_SYMBOLS
  detail::OpenGroup group_;
  std::string header_info_;
  /// \endcond
};
}  // namespace h5
