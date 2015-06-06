/* (c) Copyright 2011-2014 Felipe Magno de Almeida
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GHTV_DFB_DIRECTFB_DELETER_HPP
#define GHTV_DFB_DIRECTFB_DELETER_HPP

#include <iostream>
#include <typeinfo>

namespace ghtv { namespace dfb {

struct directfb_deleter
{
  template <typename T>
  void operator()(T* obj) const
  {
    std::cout << "Release for " << typeid(T).name() << " 0x" << obj << std::endl;
    obj->Release(obj);
  }
};

} }

#endif
