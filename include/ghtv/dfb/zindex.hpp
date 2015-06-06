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

#ifndef GHTV_DFB_ZINDEX_HPP
#define GHTV_DFB_ZINDEX_HPP

#include <gntl/parser/libxml2/dom/color.hpp>
#ifndef GHTV_DISABLE_GSTREAMER
#include <boost/thread.hpp>
#endif

#include <map>
#include <vector>
#include <algorithm>
#include <cassert>

#include <directfb.h>

namespace ghtv { namespace dfb {

struct surface
{
  IDirectFBSurface* s;
  int x, y, w, h;
  std::size_t border_size;
  gntl::parser::libxml2::dom::color border_color;
};

inline bool operator==(surface const& lhs, surface const& rhs)
{
  return lhs.s == rhs.s && lhs.x == rhs.x && lhs.y == rhs.y
    && lhs.w == rhs.w && lhs.h == rhs.h
    && lhs.border_size == rhs.border_size && lhs.border_color == rhs.border_color;
}

inline bool operator!=(surface const& lhs, surface const& rhs)
{
  return !(lhs == rhs);
}

struct zindex
{
  std::map<int, std::vector<surface> > zindex_map;
  IDirectFB* directfb;
#ifndef GHTV_DISABLE_GSTREAMER
  IDirectFBSurface* cache_surface;
  bool cached;
  boost::mutex mutex;
#endif

  zindex(IDirectFB* directfb) : directfb(directfb)
#ifndef GHTV_DISABLE_GSTREAMER
                              , cache_surface(0), cached(false)
#endif
  {}

  ~zindex()
  {
#ifndef GHTV_DISABLE_GSTREAMER
    if(cache_surface)
      cache_surface->Release(cache_surface);
#endif
  }

  void add_surface(surface s, int zindex)
  {
#ifndef GHTV_DISABLE_GSTREAMER
    boost::unique_lock<boost::mutex> lock(mutex);
#endif
    std::vector<surface>& vector = zindex_map[zindex];
    vector.push_back(s);
#ifndef GHTV_DISABLE_GSTREAMER
    cached = false;
#endif
  }

  void replace_surface(surface new_, surface old, int zindex)
  {
#ifndef GHTV_DISABLE_GSTREAMER
    boost::unique_lock<boost::mutex> lock(mutex);
#endif
    std::vector<surface>& vector = zindex_map[zindex];
    std::vector<surface>::iterator i
      = std::find(vector.begin(), vector.end(), old);
    assert(i != vector.end());
    *i = new_;
#ifndef GHTV_DISABLE_GSTREAMER
    cached = false;
#endif
  }

  void remove_surface(surface s, int zindex)
  {
#ifndef GHTV_DISABLE_GSTREAMER
    boost::unique_lock<boost::mutex> lock(mutex);
#endif
    std::vector<surface>& vector = zindex_map[zindex];
    std::vector<surface>::iterator i
      = std::find(vector.begin(), vector.end(), s);
    assert(i != vector.end());
    vector.erase(i);
#ifndef GHTV_DISABLE_GSTREAMER
    cached = false;
#endif
  }

  void draw(IDirectFBSurface* surface)
  {
    std::cout << "drawing" << std::endl;
#ifndef GHTV_DISABLE_GSTREAMER
    boost::unique_lock<boost::mutex> lock(mutex);
#endif
#ifndef GHTV_DISABLE_GSTREAMER
    if(cached)
    {
      std::cout << "blitting cached surface" << std::endl;
      assert(cache_surface != 0);
      surface->Blit(surface, cache_surface, 0, 0, 0);
    }
    else
#endif
    {
      std::cout << "cached surface doesn't exist or is dirty" << std::endl;
      IDirectFBSurface* destination = surface;
#ifndef GHTV_DISABLE_GSTREAMER
      if(!cache_surface)
      {
        std::cout << "creating cache surface" << std::endl;
        DFBSurfaceDescription description
          = {static_cast<DFBSurfaceDescriptionFlags>(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT)};
        surface->GetSize(surface, &description.width, &description.height);
        surface->GetPixelFormat(surface, &description.pixelformat);
        directfb->CreateSurface(directfb, &description, &cache_surface);
        destination = cache_surface;
      }
      else
        destination = cache_surface;
      
      if(!cache_surface)
#endif
        surface->SetBlittingFlags(surface, DSBLIT_BLEND_ALPHACHANNEL);
#ifndef GHTV_DISABLE_GSTREAMER
      else
      {
        cache_surface->Clear(cache_surface, 0u, 0u, 0u, 0);
        cache_surface->SetBlittingFlags(cache_surface, DSBLIT_BLEND_ALPHACHANNEL);
        surface->SetBlittingFlags(surface, DSBLIT_NOFX);
      }
#endif

      for(std::map<int, std::vector<dfb::surface> >::const_iterator
            first = zindex_map.begin(), last = zindex_map.end()
            ; first != last; ++first)
        for(std::vector<dfb::surface>::const_iterator sfirst = first->second.begin()
              , slast = first->second.end(); sfirst != slast; ++sfirst)
        {
          std::cout << "blitting image to primary_surface/cache_surface" << std::endl;
          if(sfirst->border_size)
          {
            destination->SetColor(destination, sfirst->border_color.r, sfirst->border_color.g
                                  , sfirst->border_color.b, 255u);
            
            destination->FillRectangle(destination, sfirst->x, sfirst->y, sfirst->w, sfirst->border_size);
            destination->FillRectangle(destination, sfirst->x, sfirst->y, sfirst->border_size, sfirst->h);
            destination->FillRectangle(destination, sfirst->x
                                       , sfirst->y + sfirst->h - sfirst->border_size
                                       , sfirst->w, sfirst->border_size);
            destination->FillRectangle(destination, sfirst->x+sfirst->w-sfirst->border_size
                                       , sfirst->y, sfirst->border_size, sfirst->h);
          }
          DFBRectangle rectangle = {sfirst->x + sfirst->border_size, sfirst->y + sfirst->border_size
                                    , sfirst->w - 2*sfirst->border_size
                                    , sfirst->h - 2*sfirst->border_size};
          std::cout << "rectangle: {x: " << rectangle.x << " y: " << rectangle.y
                    << " w: " << rectangle.w << " h: " << rectangle.h << "}" << std::endl;
          destination->StretchBlit(destination, sfirst->s, 0, &rectangle);
        }

#ifndef GHTV_DISABLE_GSTREAMER
      if(cache_surface)
      {
        std::cout << "blitting cache_surface to primary_surface" << std::endl;
        cached = true;
        surface->Blit(surface, cache_surface, 0, 0, 0);
      }
#endif
    }
    std::cout << "finished drawing" << std::endl;
  }
};


} }

#endif
