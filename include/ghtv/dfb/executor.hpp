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

#ifndef GHTV_DFB_EXECUTOR_HPP
#define GHTV_DFB_EXECUTOR_HPP

#include <ghtv/dfb/directfb_deleter.hpp>
#include <ghtv/dfb/zindex.hpp>

#include <gntl/concept/structure/executor.hpp>
#include <gntl/algorithm/structure/media/dimensions.hpp>
#include <gntl/parser/libxml2/dom/xml_string.hpp>
#include <gntl/parser/libxml2/dom/color.hpp>

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>

#include <iostream>

#include <directfb.h>
#ifndef GHTV_DISABLE_GSTREAMER
#undef __no_instrument_function__
#include <gst/gst.h>
#endif

namespace ghtv { namespace dfb {

struct executor_owner : boost::noncopyable
{
  boost::shared_ptr<IDirectFB> dfb;
  boost::shared_ptr<IDirectFBDisplayLayer> primary_layer;
  boost::shared_ptr<IDirectFBSurface> surface;
  boost::filesystem::path root_path;
  boost::shared_ptr<IDirectFBVideoProvider> video_provider;
  boost::optional<boost::filesystem::path> source_path;
  gntl::algorithm::structure::media::dimensions playing_dimensions;
  boost::posix_time::ptime start_time;
  std::size_t border_size;
  gntl::parser::libxml2::dom::color border_color;
  zindex* zindex_;

  executor_owner(boost::shared_ptr<IDirectFB> dfb
                 , boost::shared_ptr<IDirectFBDisplayLayer> primary_layer
                 , boost::filesystem::path root_path
                 , zindex& zindex_)
    : dfb(dfb), primary_layer(primary_layer)
    , root_path(root_path), border_size(0)
    , zindex_(&zindex_)
  {}

  void stop()
  {
    source_path = boost::none;
    std::cout << "closing window" << std::endl;
    if(video_provider)
      video_provider->Stop(video_provider.get());

    if(surface)
    {
      ghtv::dfb::surface s = {surface.get(), playing_dimensions.x, playing_dimensions.y
                              , playing_dimensions.width, playing_dimensions.height
                              , border_size, border_color};
      zindex_->remove_surface(s, playing_dimensions.zindex);
      surface.reset();
    }
  }

  ~executor_owner()
  {
    stop();
  }
};

} }

namespace gntl { namespace concept { namespace structure {

template <>
struct executor_traits<boost::shared_ptr<ghtv::dfb::executor_owner> >
{
  typedef boost::mpl::true_ is_executor;
  typedef boost::shared_ptr<ghtv::dfb::executor_owner> executor;
  typedef boost::posix_time::time_duration time_duration;
  typedef gntl::parser::libxml2::dom::color color_type;
  typedef gntl::parser::libxml2::dom::xml_string<> interface_type;
  typedef gntl::parser::libxml2::dom::xml_string<> component_identifier;

  typedef boost::mpl::vector
  <void(executor, gntl::parser::libxml2::dom::xml_string<>
        , boost::optional<std::string>
        , gntl::algorithm::structure::media::dimensions
        , component_identifier)> start_function_overloads;
  typedef boost::mpl::vector
  <void(executor)> stop_function_overloads;
  typedef boost::mpl::vector
  <void(executor)> pause_function_overloads;
  typedef boost::mpl::vector
  <void(executor)> resume_function_overloads;
  typedef boost::mpl::vector
  <void(executor)> abort_function_overloads;

  static void add_border(executor& e, std::size_t border_width
                         , gntl::parser::libxml2::dom::color border_color)
  {
    std::cout << "add border" << std::endl;

    ghtv::dfb::surface sold = {e->surface.get(), e->playing_dimensions.x
                               , e->playing_dimensions.y
                               , e->playing_dimensions.width, e->playing_dimensions.height
                               , e->border_size, e->border_color};
    e->border_size = border_width;
    e->border_color = border_color;
    if(e->surface)
    {
      ghtv::dfb::surface snew = {e->surface.get(), e->playing_dimensions.x
                                 , e->playing_dimensions.y
                                 , e->playing_dimensions.width, e->playing_dimensions.height
                                 , e->border_size, e->border_color};

      e->zindex_->replace_surface(snew, sold, e->playing_dimensions.zindex);
    }
  }

  static void remove_border(executor& e)
  {
    std::cout << "remove border" << std::endl;

    ghtv::dfb::surface sold = {e->surface.get(), e->playing_dimensions.x
                               , e->playing_dimensions.y
                               , e->playing_dimensions.width, e->playing_dimensions.height
                               , e->border_size, e->border_color};

    e->border_size = 0;

    if(e->surface)
    {
      ghtv::dfb::surface snew = {e->surface.get(), e->playing_dimensions.x
                                 , e->playing_dimensions.y
                                 , e->playing_dimensions.width, e->playing_dimensions.height
                                 , e->border_size, e->border_color};
      e->zindex_->replace_surface(snew, sold, e->playing_dimensions.zindex);
    }
  }

  static void start_source(executor& e)
  {
    std::cout << "(executor) Trying to open " << *e->source_path << " zindex: "
              << e->playing_dimensions.zindex << std::endl;
    assert(!e->surface);
#if BOOST_VERSION >= 104600
    std::string path_string = e->source_path->string();
#else
    std::string path_string = e->source_path->external_file_string();
#endif
    if(boost::algorithm::iends_with (path_string, ".png")
       || boost::algorithm::iends_with (path_string, ".jpg"))
    {
      boost::shared_ptr<IDirectFBImageProvider> provider;
      {
        IDirectFBImageProvider* p = 0;
        e->dfb->CreateImageProvider(e->dfb.get(), path_string.c_str(), &p);
        if(p)
          provider.reset(p, ghtv::dfb::directfb_deleter());
      }

      if(provider)
      {
        boost::shared_ptr<IDirectFBSurface> surface;
        {
          DFBSurfaceDescription description;      
          provider->GetSurfaceDescription (provider.get(), &description);
          description.flags = static_cast<DFBSurfaceDescriptionFlags>
            (description.flags | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
          description.width = e->playing_dimensions.width;
          description.height = e->playing_dimensions.height;
          description.pixelformat = DSPF_ARGB8565;
          IDirectFBSurface* p = 0;
          if(e->dfb->CreateSurface(e->dfb.get(), &description, &p) != DFB_OK)
          {
            std::cout << "Couldn't create surface" << std::endl;
            throw std::runtime_error("Couldn't create surface");
          }
          surface.reset(p, ghtv::dfb::directfb_deleter());

          provider->RenderTo(provider.get(), surface.get(), 0);
        }
        
        ghtv::dfb::surface snew
          = {surface.get(), e->playing_dimensions.x, e->playing_dimensions.y
             , e->playing_dimensions.width, e->playing_dimensions.height
             , e->border_size, e->border_color};
        e->zindex_->add_surface(snew, e->playing_dimensions.zindex);
        std::swap(e->surface, surface);
      }
      else
        std::cout << "No image provider found" << std::endl;
    }
    else if(boost::algorithm::iends_with (path_string, ".mp4"))
    {
      std::cout << "Video provider created" << std::endl;
      // DFBRectangle rectangle = {e->playing_dimensions.x, e->playing_dimensions.y
      //                           , e->playing_dimensions.width, e->playing_dimensions.height};

      // IDirectFBSurface* p = 0;
      // e->primary_layer->GetSurface(e->primary_layer.get(), &p);
      // if(p)
      // {
//         boost::shared_ptr<IDirectFBSurface> primary(p, ghtv::dfb::directfb_deleter());
//         GstElement* pipeline = gst_pipeline_new(0);
//         assert(pipeline != 0);

// 	GstElement* decode = gst_element_factory_make("playbin2", 0);
//         assert(decode != 0);

//         GstElement* sink = gst_element_factory_make ("dfbvideosink", "sink");
//         assert(sink != 0);
//         g_object_set(sink, "surface", p, 0);

// #if BOOST_VERSION >= 104600
//         std::string path_string = e->source_path->string();
// #else
//         std::string path_string = e->source_path->external_file_string();
// #endif
//         g_object_set(decode, "uri", ("file:///" + path_string).c_str(), 0);
//         g_object_set(decode, "video-sink", sink, 0);

//         gst_bin_add_many (GST_BIN(pipeline), decode, 0);

//         gst_element_set_state (pipeline, GST_STATE_PLAYING);

//         GMainLoop* loop = g_main_loop_new (NULL, FALSE);

//         //g_timeout_add (20000, get_me_out, NULL);
//         g_main_loop_run (loop);

//         gst_element_set_state (pipeline, GST_STATE_NULL);
//         g_main_loop_unref (loop);
      // }
      // else
      //   throw std::runtime_error("Couldn't create surface for video");
    }
  }

  static void start_area(executor& e, component_identifier)
  {
  }
  static void start(executor& e, gntl::parser::libxml2::dom::xml_string<> source
                    , boost::optional<std::string> interface_
                    , gntl::algorithm::structure::media::dimensions dim
                    , component_identifier)
  {
    if(e->surface.get())
      e->stop();

    std::cout << "(executor) starting ";
    if(source != "")
      std::cout << source;
    else
      std::cout << "[no-src]";
    std::cout << " with dimensions " << dim << std::endl;

    std::string new_source(source.c_str());
    std::replace(new_source.begin(), new_source.end(), '\\', '/');
    e->source_path = e->root_path / new_source;
    e->playing_dimensions = dim;

    start_source(e);

    std::cout << "(executor) start finish" << std::endl;
    e->start_time = boost::posix_time::microsec_clock::local_time();
    std::cout << "saved start time " << e->start_time << std::endl;
  }

  static void stop(executor& e)
  {
    e->stop();
  }

  static void pause(executor& e) {}
  static void resume(executor& e) {}
  static void abort(executor& e)
  {
    e->stop();
  }

  static void area_time_begin(executor& e, component_identifier i, time_duration)
  {
  }
  static void area_time_end(executor& e, component_identifier i, time_duration)
  {
  }
  static void area_time_begin_end(executor& e, component_identifier i, time_duration, time_duration)
  {
  }
  static void area_frame_begin(executor& e, component_identifier i, int)
  {
  }
  static void area_frame_end(executor& e, component_identifier i, int)
  {
  }
  static void area_frame_begin_end(executor& e, component_identifier i, int, int)
  {
  }
  static void area_npt_begin(executor& e, component_identifier i, unsigned int)
  {
  }
  static void area_npt_end(executor& e, component_identifier i, unsigned int)
  {
  }
  static void area_npt_begin_end(executor& e, component_identifier i, unsigned int, unsigned int)
  {
  }
  static void explicit_duration(executor& e, time_duration)
  {
  }
  static bool start_set_property(executor& e, std::string, std::string)
  {
    return false;
  }
  static bool commit_set_property(executor& e, std::string)
  {
    return false;
  }
  static bool wants_keys(executor const& e)
  {
    return false;
  }
};

} } }

#endif
