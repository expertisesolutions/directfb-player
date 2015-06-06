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

#include <ghtv/dfb/executor.hpp>

#include <gntl/parser/libxml2/dom/document.hpp>
#include <gntl/parser/libxml2/dom/xml_document.hpp>
#include <gntl/parser/libxml2/dom/head.hpp>
#include <gntl/parser/libxml2/dom/document.hpp>
#include <gntl/parser/libxml2/dom/context.hpp>
#include <gntl/algorithm/structure/context/evaluate_links.hpp>
#include <gntl/structure/composed/context.hpp>
#include <gntl/structure/composed/media.hpp>
#include <gntl/structure/composed/document.hpp>
#include <gntl/structure/composed/link.hpp>
#include <gntl/structure/composed/descriptor.hpp>
#include <gntl/algorithm/structure/document.hpp>
#include <gntl/algorithm/structure/media.hpp>
#include <gntl/algorithm/structure/context.hpp>
#include <gntl/algorithm/structure/link.hpp>
#include <gntl/algorithm/structure/condition/evaluate_bound_simple_condition.hpp>
#include <gntl/algorithm/structure/context/start.ipp>
#include <gntl/algorithm/structure/context/start_action_traits.ipp>
#include <gntl/algorithm/structure/context/stop.ipp>
#include <gntl/algorithm/structure/context/stop_action_traits.ipp>
#include <gntl/algorithm/structure/context/pause.ipp>
#include <gntl/algorithm/structure/context/pause_action_traits.ipp>
#include <gntl/algorithm/structure/context/resume.ipp>
#include <gntl/algorithm/structure/context/resume_action_traits.ipp>
#include <gntl/algorithm/structure/context/abort.ipp>
#include <gntl/algorithm/structure/context/abort_action_traits.ipp>
#include <gntl/algorithm/structure/port/start.ipp>
#include <gntl/algorithm/structure/port/start_action_traits.ipp>
#include <gntl/algorithm/structure/component/start.ipp>
#include <gntl/algorithm/structure/component/stop.ipp>
#include <gntl/algorithm/structure/component/pause.ipp>
#include <gntl/algorithm/structure/component/resume.ipp>
#include <gntl/algorithm/structure/component/abort.ipp>
#include <gntl/algorithm/structure/media/select.hpp>
#include <gntl/algorithm/structure/media/select.ipp>
#include <gntl/algorithm/structure/media/start.ipp>
#include <gntl/algorithm/structure/media/start_action_traits.ipp>
#include <gntl/algorithm/structure/media/stop.ipp>
#include <gntl/algorithm/structure/media/stop_action_traits.ipp>
#include <gntl/algorithm/structure/media/resume.ipp>
#include <gntl/algorithm/structure/media/resume_action_traits.ipp>
#include <gntl/algorithm/structure/media/pause.ipp>
#include <gntl/algorithm/structure/media/pause_action_traits.ipp>
#include <gntl/algorithm/structure/media/abort.ipp>
#include <gntl/algorithm/structure/media/abort_action_traits.ipp>
#include <gntl/algorithm/structure/media/set.ipp>
#include <gntl/algorithm/structure/media/set_action_traits.ipp>
#include <gntl/algorithm/structure/switch/start.ipp>
#include <gntl/algorithm/structure/switch/start_action_traits.ipp>
#include <gntl/algorithm/structure/switch/stop.ipp>
#include <gntl/algorithm/structure/switch/stop_action_traits.ipp>
#include <gntl/algorithm/structure/switch/abort.ipp>
#include <gntl/algorithm/structure/switch/abort_action_traits.ipp>
#include <gntl/algorithm/parser/document.hpp>
#include <gntl/ref.hpp>

#include <sdsmcc/save_dsmcc.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <iostream>

#include <directfb.h>
#ifndef GHTV_DISABLE_GSTREAMER
#include <gst/gst.h>
#endif

#include <cassert>
#include <unistd.h>
#include <limits>
#include <new>

#if 0
#include <execinfo.h>

std::size_t current_allocation_size = 0;
std::size_t allocations = 0;
std::size_t outstanding_allocations = 0;
std::size_t maximum_allocation_size = 0;

void* operator new(std::size_t size) throw (std::bad_alloc)
{
  // std::cout << "(new) current_allocations_size " << current_allocation_size << std::endl;
  // std::cout << "(new) Size: " << size << std::endl;
  
  void* p = ::malloc(size+sizeof(std::size_t));
  if(!p)
    throw std::bad_alloc();
  std::memcpy(p, &size, sizeof(std::size_t));
  ++allocations; ++outstanding_allocations;
  current_allocation_size += size;
  if(current_allocation_size > maximum_allocation_size)
  {
    maximum_allocation_size = current_allocation_size;
    printf("Reached new maximum %d\n", (int)maximum_allocation_size);
    void* stack[128];
    backtrace(stack, 128);
    backtrace_symbols_fd(stack, 128, stdout);
  }
  // std::cout << "(new) NEW current_allocations_size " << current_allocation_size << std::endl;
  // std::cout << "Allocated pointer " << (void*)(((char*)p) + sizeof(std::size_t)) << std::endl;
  return ((char*)p) + sizeof(std::size_t);
}

void* operator new[](std::size_t size) throw (std::bad_alloc)
{
  return ::operator new(size);
}

void operator delete(void* p)
{
  // std::cout << "Freeing pointer " << p << std::endl;
  // std::cout << "(delete) current_allocations_size " << current_allocation_size << std::endl;
  p = (char*)p - sizeof(std::size_t);
  std::size_t size = 0;
  std::memcpy(&size, p, sizeof(std::size_t));
  // std::cout << "(delete) Size: " << size << std::endl;
  current_allocation_size -= size;
  // std::cout << "(delete) NEW current_allocations_size " << current_allocation_size << std::endl;
  --outstanding_allocations;
  ::free(p);
}

void operator delete[](void* p)
{
  ::operator delete (p);
}

void print_memory_info()
{
  std::cout << "Number of allocations: " << allocations << std::endl;
  std::cout << "Number of outstanding allocations: " << outstanding_allocations << std::endl;
  std::cout << "Current allocation size: " << current_allocation_size << std::endl;
  std::cout << "Maximum allocation size: " << maximum_allocation_size << std::endl;
}
#endif

struct presentation_factory
{
  boost::shared_ptr<IDirectFB> dfb;
  boost::shared_ptr<IDirectFBDisplayLayer> primary_layer;
  boost::filesystem::path root_path;
  ghtv::dfb::zindex* zindex_;

  typedef boost::shared_ptr<ghtv::dfb::executor_owner> executor_type;
  typedef gntl::structure::composed::descriptor
    <gntl::parser::libxml2::dom::descriptor, std::string> descriptor_type;
  typedef gntl::structure::composed::presentation<executor_type, descriptor_type> presentation_type;
  typedef presentation_type result_type;
  result_type operator()(std::string media, descriptor_type d) const
  {
    result_type r(executor_type(new ghtv::dfb::executor_owner(dfb, primary_layer, root_path, *zindex_)), d);
    return r;
  }
};

void read_ncl(std::auto_ptr<gntl::parser::libxml2::dom::xml_document>& xmldocument
              , boost::filesystem::path ncl_file)
{
  std::vector<char> file_buffer;
  {
    boost::filesystem::ifstream file(ncl_file);

    if(!file.is_open())
    {
      std::cout << "Could not open " << ncl_file << " file" << std::endl;
      throw std::runtime_error("Could not open file");
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
      
    if(size == 0)
    {
      std::cout << "File " << ncl_file << " is empty" << std::endl;
      throw std::runtime_error("File is empty");
    }
          
    file_buffer.resize(size+1);
    file.rdbuf()->sgetn(&file_buffer[0], size);
  }    
  std::cout << "Parsing " << ncl_file << std::endl;
  ::xmlDocPtr xmldoc = xmlParseDoc
      (static_cast<const xmlChar*>
       (static_cast<const void*>(&file_buffer[0])));
  std::cout << "Parsed " << ncl_file << std::endl;
  file_buffer.clear();
  std::cout << "Released memory for " << ncl_file << std::endl;
  if(!boost::filesystem::exists(ncl_file))
  {
    std::cout << "File " << ncl_file << " doesn't exist" << std::endl;
    throw std::runtime_error("File doesnt exist");
  }

  xmldocument.reset(new gntl::parser::libxml2::dom::xml_document(xmldoc)); // gets ownership
}

#ifndef GHTV_DISABLE_GSTREAMER
struct post_render_info
{
  IDirectFBSurface* primary;
  ghtv::dfb::zindex& zindex_;
};

void post_render(void* surface_void)
{
  std::cout << "Post render" << std::endl;
  assert(surface_void != 0);
  post_render_info* info = static_cast<post_render_info*>(surface_void);
  info->zindex_.draw(info->primary);
  info->primary->Flip(info->primary, 0, DSFLIP_WAIT);
}
#endif

typedef std::string document_uri_type;
typedef gntl::structure::composed::document
 <gntl::parser::libxml2::dom::document, presentation_factory
  , document_uri_type> structure_document;

void import_document(document_uri_type document_uri
                     , std::map<std::string, structure_document>* imported_documents
                     , boost::filesystem::path root_path
                     , std::vector<boost::shared_ptr<gntl::parser::libxml2::dom::xml_document> >& xml_documents
                     , presentation_factory factory)
{
  boost::filesystem::path filename = root_path / document_uri;
  std::cout << "Should import " << filename << std::endl;

  std::auto_ptr<gntl::parser::libxml2::dom::xml_document> xml;
  read_ncl(xml, filename);
  boost::shared_ptr<gntl::parser::libxml2::dom::xml_document> xml_imported(xml);
  xml_documents.push_back(xml_imported);
  gntl::parser::libxml2::dom::document d (xml_imported->root ());
  
  std::map<document_uri_type, structure_document> documents;
  gntl::algorithm::parser::document::for_each_import_document_uri
    (gntl::ref(d), boost::bind(&::import_document, _1, &documents, root_path, boost::ref(xml_documents), factory));
  
  imported_documents->insert
    (std::make_pair(document_uri, structure_document(d, document_uri, factory, documents)));
}

#ifndef GHTV_DISABLE_GSTREAMER
namespace ghtv { namespace dfb {

gboolean video_sink_init(GstPlugin*);

} }
#endif

int main(int argc, char* argv[])
{
  try
  {
    DirectFBInit(&argc, &argv);

#ifndef GHTV_DISABLE_GSTREAMER
    gst_init(&argc, &argv);
    gst_plugin_register_static(GST_VERSION_MAJOR
                               , GST_VERSION_MINOR
                               , "ghtv_video_sink"
                               , "Some description"
                               , &ghtv::dfb::video_sink_init
                               , "1.0"
                               , "Proprietary"
                               , "XXX"
                               , "XXX", "XXX");
#endif

    int width = 1280, height = 720;
    boost::filesystem::path ncl_file, image_file;
#ifndef GHTV_DISABLE_GSTREAMER
    boost::filesystem::path video_file;
#endif
    bool remote = false;
    std::auto_ptr<gntl::parser::libxml2::dom::xml_document> xmldocument;
    {
      boost::program_options::options_description descriptions("Allowed options");
      descriptions.add_options()
        ("help", "produce help messages")
        ("ncl", boost::program_options::value<std::string>(), "NCL file to play")
        ("frequency", boost::program_options::value<unsigned int>(), "Frequency do download app")
        ("width", boost::program_options::value<int>(), "width resolution")
        ("height", boost::program_options::value<int>(), "height resolution")
        ("remote", boost::program_options::value<bool>(), "If application should use remote control for input")
#ifndef GHTV_DISABLE_GSTREAMER
        ("video", boost::program_options::value<std::string>(), "Path for principal video")
#endif
        ("image", boost::program_options::value<std::string>(), "Path for background image")
        ;

      boost::program_options::variables_map vm;
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, descriptions), vm);
      boost::program_options::notify(vm);
      
      if(vm.count("help") > 0)
      {
        std::cout << descriptions << std::endl;
        return 1;
      }

      if(vm.count("ncl") > 0)
      {
        std::cout << "Loading " << ncl_file << " to memory" << std::endl;
        ncl_file = vm["ncl"].as<std::string>();
        read_ncl(xmldocument, ncl_file);
      }
      else if(vm.count("frequency"))
      {
        std::cout << "Starting download" << std::endl;
        boost::filesystem::path temporary_directory;
#if BOOST_VERSION >= 104600
        do
        {
          temporary_directory = boost::filesystem::temp_directory_path()
            / boost::filesystem::unique_path();
        } while(!boost::filesystem::create_directory(temporary_directory));
#else
        {
          char template_[] = "temporaryXXXXXX";
          char* temp = mkdtemp(template_);
          temporary_directory = temp;
        }
#endif
        sdsmcc::save_dsmcc(vm["frequency"].as<unsigned int>(), temporary_directory, 0);
        std::cout << "Finished download" << std::endl;
        if(boost::filesystem::exists(temporary_directory / "directory0/SAOSILrev18/saosil.ncl"))
        {
          std::cout << "exists saosil.ncl" << std::endl;
          ncl_file = temporary_directory / "directory0/SAOSILrev18/saosil.ncl";
        }
        else if(boost::filesystem::exists(temporary_directory / "directory0/main.ncl"))
        {
          ncl_file = temporary_directory / "directory0/main.ncl";
        }
        else
        {
          std::cout << "Unknown entry point ncl file" << std::endl;
          return -1;
        }

        read_ncl(xmldocument, ncl_file);
      }
      else
      {
        std::cout << "You must specify the --ncl parameter or --frequency" << std::endl << descriptions << std::endl;
        return -1;
      }
      if(vm.count("width") > 0)
      {
        width = vm["width"].as<int>();
      }
      if(vm.count("height") > 0)
      {
        height = vm["height"].as<int>();
      }
      if(vm.count("remote") > 0)
      {
        remote = vm["remote"].as<bool>();
      }
#ifndef GHTV_DISABLE_GSTREAMER
      if(vm.count("video") > 0)
      {
        video_file = vm["video"].as<std::string>();
      }
      else
#endif
      if(vm.count("image") > 0)
      {
        image_file = vm["image"].as<std::string>();
      }
    }

    
    boost::shared_ptr<IDirectFB> dfb;
    {
      IDirectFB* p = 0;
      DirectFBCreate(&p);
      dfb.reset(p, ghtv::dfb::directfb_deleter());
    }

    if(dfb->SetCooperativeLevel(dfb.get(), DFSCL_FULLSCREEN) != DFB_OK)
    {
      std::cout << "Couldn't setCooperativeLevel FULLSCREEN" << std::endl;
    }

    std::cout << "screen size width: " << width << " height: " << height << std::endl;

    boost::shared_ptr<IDirectFBDisplayLayer> layer;
    {
      IDirectFBDisplayLayer* p = 0;
      dfb->GetDisplayLayer(dfb.get(), DLID_PRIMARY, &p);
      layer.reset(p, ghtv::dfb::directfb_deleter());
    }
    if(layer->SetCooperativeLevel(layer.get(), DLSCL_EXCLUSIVE) != DFB_OK)
    {
      std::cout << "Couldn't setCooperativeLevel EXCLUSIVE" << std::endl;
      return -1;
    }

    {
      DFBDisplayLayerConfig config;
      config.flags = static_cast<DFBDisplayLayerConfigFlags>
        (DLCONF_WIDTH | DLCONF_HEIGHT | /* DLCONF_PIXELFORMAT | */DLCONF_SURFACE_CAPS | DLCONF_BUFFERMODE);
      config.width = width;
      config.height = height;
      //config.pixelformat = DSPF_ARGB;
      config.surface_caps = static_cast<DFBSurfaceCapabilities>(DSCAPS_DOUBLE | DSCAPS_PRIMARY);
      config.buffermode = DLBM_BACKVIDEO;
      if(layer->SetConfiguration(layer.get(), &config) != DFB_OK)
      {
        std::cout << "Failed setting config" << std::endl;
        return -1;
      }
    }

    boost::shared_ptr<IDirectFBSurface> primary;
    {
      {
        IDirectFBSurface* p = 0;
        layer->GetSurface(layer.get(), &p);
        primary.reset(p, ghtv::dfb::directfb_deleter());
      }
      assert(!!primary);

      std::cout << "Primary surface size width: " << width << " height: " << height << std::endl;

      DFBRegion* r = 0;
      primary->SetClip(primary.get(), r);
      primary->Clear(primary.get(), 0u, 0u, 0u, 0);
      primary->Flip(primary.get(), 0, DSFLIP_WAITFORSYNC);
      primary->Clear(primary.get(), 0u, 0u, 0u, 0);
    }

    ghtv::dfb::zindex zindex_(dfb.get());
    boost::shared_ptr<IDirectFBSurface> background_surface;
#ifndef GHTV_DISABLE_GSTREAMER
    bool has_main_video = false;
    post_render_info render_info = {primary.get(), zindex_};
    if(video_file != boost::filesystem::path())
    {
      std::cout << "Opening video_file " << video_file << std::endl;
      //DFBRectangle rectangle = {0, 0, width, height};

      GstElement* pipeline = gst_pipeline_new(0);
      GstElement* sink_pipeline = gst_pipeline_new(0);
      assert(pipeline != 0);
      assert(sink_pipeline != 0);
      GstElement* decode = gst_element_factory_make("playbin2", 0);
      assert(decode != 0);
      GstElement* converter = gst_element_factory_make("TIViddec2", 0);
      assert(converter != 0);
      GstElement* scaler = gst_element_factory_make("TIVidResize", "scaler");
      assert(scaler != 0);
      // GstElement* conv = gst_element_factory_make ("TIC6xColorspace", "conv");
      // assert(conv != 0);
      // g_object_set(G_OBJECT(scaler), "caps"
      //              , gst_caps_new_simple("video/x-raw-rgb"
      //                                    // , "bpp", G_TYPE_INT, 16
      //                                    // , "depth", G_TYPE_INT, 16
      //                                    , "width", G_TYPE_INT, 1280
      //                                    , "height", G_TYPE_INT, 720
      //                                    , 0)
      //              , 0);
      // g_object_set(G_OBJECT(scaler), "width", 1280, 0);
      // g_object_set(G_OBJECT(scaler), "height", 720, 0);
      GstElement* sink = gst_element_factory_make ("ghtv_video_sink", "sink");
      assert(sink != 0);
      void* void_nil = NULL;
      g_object_set(sink, "surface", static_cast<void*>(primary.get()), void_nil);
      g_object_set(sink, "post-render-callback", reinterpret_cast<void*>(&::post_render)
                   , void_nil);
      g_object_set(sink, "callback-data", static_cast<void*>(&render_info), void_nil);
      
#if BOOST_VERSION >= 104600
      std::string path_string = video_file.string();
#else
      std::string path_string = video_file.external_file_string();
#endif

      gst_bin_add_many (GST_BIN(sink_pipeline), converter
                        , scaler, /*conv,*/ sink, void_nil);
      gst_element_link_many(converter, scaler, /*conv,*/ sink, void_nil);

      {
        GstPad* pad = gst_element_get_static_pad(converter, "sink");
        assert(pad != 0);
        gst_element_add_pad(sink_pipeline, gst_ghost_pad_new("converter", pad));
        gst_object_unref(GST_OBJECT(pad));
      }

      g_object_set(decode, "uri", ("file:///" + path_string).c_str(), void_nil);
      g_object_set(decode, "video-sink", sink_pipeline, void_nil);
      gst_bin_add_many (GST_BIN(pipeline), decode, void_nil);
      gst_element_set_state (pipeline, GST_STATE_PLAYING);

      has_main_video = true;

      std::cout << "Prep'ed video file " << video_file << std::endl;
    }
    else
#endif
    if(image_file != boost::filesystem::path())
    {
#if BOOST_VERSION >= 104600
    std::string path_string = image_file.string();
#else
    std::string path_string = image_file.external_file_string();
#endif
      boost::shared_ptr<IDirectFBImageProvider> provider;
      {
        IDirectFBImageProvider* p = 0;
        dfb->CreateImageProvider(dfb.get(), path_string.c_str(), &p);
        if(p)
          provider.reset(p, ghtv::dfb::directfb_deleter());
      }
      if(provider)
      {
        {
          DFBSurfaceDescription description;      
          provider->GetSurfaceDescription (provider.get(), &description);
          // description.flags = static_cast<DFBSurfaceDescriptionFlags>
          //   (description.flags | DSDESC_WIDTH | DSDESC_HEIGHT);
          // description.width = e->playing_dimensions.width;
          // description.height = e->playing_dimensions.height;
          IDirectFBSurface* p = 0;
          if(dfb->CreateSurface(dfb.get(), &description, &p) != DFB_OK)
          {
            std::cout << "Couldn't create surface" << std::endl;
            throw std::runtime_error("Couldn't create surface");
          }
          background_surface.reset(p, ghtv::dfb::directfb_deleter());

          provider->RenderTo(provider.get(), background_surface.get(), 0);
        }
        gntl::parser::libxml2::dom::color border_color;
        ghtv::dfb::surface snew
          = {background_surface.get(), 0u, 0u
             , width, height, 0, border_color};
        zindex_.add_surface(snew, (std::numeric_limits<int>::min)());
      }
    }

    {
      gntl::parser::libxml2::dom::document d (xmldocument->root ());
      typedef gntl::concept::parser::body_traits
        <gntl::parser::libxml2::dom::document::body_type> body_traits;
      typedef gntl::concept::parser::document_traits
        <gntl::parser::libxml2::dom::document> document_traits;
      typedef gntl::concept::parser::head_traits
        <gntl::parser::libxml2::dom::head> head_traits;

      boost::filesystem::path root_path = ncl_file.parent_path();

      presentation_factory e_factory = {dfb, layer, root_path, &zindex_};

      typedef gntl::parser::libxml2::dom::document
        parser_document;
      typedef gntl::parser::libxml2::dom::head
        parser_head;
      typedef gntl::structure::composed::descriptor
        <gntl::parser::libxml2::dom::descriptor
         , document_uri_type>
        descriptor_type;

      gntl::parser::libxml2::dom::descriptor_base descriptor_base;
      gntl::parser::libxml2::dom::head head = document_traits::head(d);

      if(head_traits::has_descriptor_base (head))
        descriptor_base = head_traits::descriptor_base (head);
      
      gntl::parser::libxml2::dom::region_base region_base;

      std::map<document_uri_type, structure_document> documents;
      std::vector<boost::shared_ptr<gntl::parser::libxml2::dom::xml_document> > xml_documents;

      gntl::algorithm::parser::document::for_each_import_document_uri
        (gntl::ref(d), boost::bind(&::import_document, _1, &documents, root_path, boost::ref(xml_documents)
                                   , e_factory));
    
      structure_document document(d
#if BOOST_VERSION >= 104600
                                  , ncl_file.string()
#else
                                  , ncl_file.filename()
#endif
                                  , e_factory
                                  , documents);

      typedef gntl::concept::structure::document_traits<structure_document> structure_document_traits;

      gntl::algorithm::structure::media::dimensions screen_dimensions
        = {0, 0, width, height, 0};

      std::cout << "NCL screen dimensions: " << screen_dimensions << std::endl;
      gntl::algorithm::structure::context::start (gntl::ref(document.body), descriptor_type()
                                                  , gntl::ref(document)
                                                  , screen_dimensions);

      std::cout << "Changed" << std::endl;

      DFBInputDeviceID device = remote?DIDID_REMOTE:DIDID_KEYBOARD;

      IDirectFBInputDevice* keyboard = 0;
      dfb->GetInputDevice (dfb.get(), device, &keyboard);
      if(!keyboard)
      {
        std::cout << "No input was found" << std::endl;
        return 0;
      }
      else
      {
        DFBInputDeviceDescription description;
        if(keyboard->GetDescription(keyboard, &description) == DFB_OK)
        {
          std::cout << "Got description: " << description.name << std::endl;
          std::cout << "Got description: " << description.vendor << std::endl;
        }
        else
          std::cout << "Failed getting description" << std::endl;
      }
      IDirectFBEventBuffer* keyboard_buffer = 0;
      keyboard->CreateEventBuffer (keyboard, &keyboard_buffer);
      if(!keyboard_buffer)
      {
        std::cout << "Couldn't create keyboard buffer" << std::endl;
        return 0;
      }

      bool running = true, dirty = true;
      // Main Loop
      while(running)
      {
        // we have nothing else to do, check events
        std::cout << "idle_function pending events: "
                  << structure_document_traits::pending_events(document) << std::endl;

        while(structure_document_traits::pending_events(document))
        {
          structure_document_traits::event_type event = structure_document_traits::top_event (document);
          std::cout << "identifier: " << event.component_identifier << std::endl;
          structure_document_traits::pop_event (document);

          try
          {
            gntl::algorithm::structure::context::evaluate_links
              (gntl::ref(document.body)
               , gntl::ref(document)
               , event, screen_dimensions);
            dirty = true;
          }
          catch(std::exception& e)
          {
            std::cout << "Catch exception: " << e.what() << std::endl;
          }

          if(event.event_ == gntl::event_enum::attribution
             && event.transition == gntl::transition_enum::starts
             && event.interface_)
          {
            typedef structure_document_traits::media_lookupable media_lookupable;
            media_lookupable lookupable = structure_document_traits::media_lookup
              (document);
            typedef gntl::concept::lookupable_traits<media_lookupable> lookupable_traits;
            typedef lookupable_traits::result_type lookup_result;

            lookup_result r = lookupable_traits::lookup(lookupable, event.component_identifier);
            typedef lookupable_traits::value_type media_type;

            if(r != lookupable_traits::not_found(lookupable))
            {
              gntl::algorithm::structure::media::commit_set(*r, *event.interface_, gntl::ref(document));

            }
          }
        }

        {
          std::cout << "Waiting for input events" << std::endl;

#if 0
          print_memory_info();
#endif

          if(
#ifndef GHTV_DISABLE_GSTREAMER
             !has_main_video &&
#endif
             keyboard_buffer->HasEvent(keyboard_buffer) == DFB_BUFFEREMPTY && dirty)
          {
            zindex_.draw(primary.get());
            primary->Flip(primary.get(), 0, DSFLIP_WAITFORSYNC);
            dirty = false;
            primary->Clear(primary.get(), 0u, 0u, 0u, 0);
          }

          DFBResult r = keyboard_buffer->WaitForEvent (keyboard_buffer);
          if(r != DFB_OK && dirty)
          {
            std::cerr << "Error waiting for key: " << r << std::endl;
          }
          else
          {
            std::cout << "Waited" << std::endl;
            DFBInputEvent event;
            if ((r = keyboard_buffer->GetEvent (keyboard_buffer, DFB_EVENT(&event))) == DFB_OK)
            {
              std::cout << "testing event" << std::endl;
              if (event.type == DIET_KEYPRESS)
              {
                std::cout << "key press keyboard event" << std::endl;
                std::string key;
                switch(event.key_symbol)
                {
                case DIKS_CURSOR_LEFT:
                  key = "CURSOR_LEFT";
                  break;
                case DIKS_CURSOR_RIGHT:
                  key = "CURSOR_RIGHT";
                  break;
                case DIKS_CURSOR_UP:
                  key = "CURSOR_UP";
                  break;
                case DIKS_CURSOR_DOWN:
                  key = "CURSOR_DOWN";
                  break;
                case DIKS_F1:
                  key = "RED";
                  break;
                case DIKS_F2:
                  key = "BLUE";
                  break;
                case DIKS_F3:
                  key = "GREEN";
                  break;
                case DIKS_F4:
                  key = "YELLOW";
                  break;
                case DIKS_F6:
                  std::cout << "Exiting application" << std::endl;
                  return 0;
                  break;
                case DIKS_ENTER:
                  key = "ENTER";
                  break;
                default:
                  std::cerr << "Unknown key symbol: " << event.key_symbol << "Unknown key id: " << event.key_id << std::endl;
                  break;
                }
                if(!key.empty())
                {
                  std::cout << "Key pressed "  << key << std::endl;
                  bool processed_current_key_master = false;
                  if(structure_document_traits::has_current_key_master(document))
                  {
                    if(key == "BACK")
                    {
                      // TODO: should unset current key master
                    }
                    else if(key != "BACK")
                    {
                      std::cout << "broadcasting key" << std::endl;
                      typedef structure_document_traits::property_type property_type;
                      typedef gntl::concept::structure::property_traits<property_type>
                        property_traits;
                      if(structure_document_traits::has_property
                         (document, "service.currentFocus")
                         && property_traits::is_integer
                         (structure_document_traits::get_property
                          (document, "service.currentFocus"))
                         && structure_document_traits::focused_media(document)
                         == structure_document_traits::current_key_master(document)
                         )
                      {
                        assert(structure_document_traits::has_focus(document));

                        typedef structure_document_traits::focus_index_presentation_lookupable
                          presentation_lookupable;
                        presentation_lookupable lookupable = structure_document_traits
                          ::focus_index_presentation_lookup(document);
                        typedef gntl::concept::lookupable_traits<presentation_lookupable>
                          lookupable_traits;
                        typedef lookupable_traits::result_type lookup_result;
                        lookup_result r = lookupable_traits::lookup
                          (lookupable
                           , property_traits::integer_value
                           (structure_document_traits::get_property
                            (document, "service.currentFocus")));

                        if(r != lookupable_traits::not_found(lookupable))
                        {
                          typedef lookupable_traits::value_type lookup_value_type;
                          lookup_value_type v = *r;
                          typedef lookup_value_type::const_iterator iterator;
                          for(iterator first = v.begin (), last = v.end ()
                                ; first != last; ++first)
                          {
                            typedef lookup_value_type::value_type value_type;
                            typedef value_type::second_type ref_presentation_type;
                            typedef gntl::unwrap_parameter<ref_presentation_type>::type
                              presentation_type;
                            typedef gntl::concept::structure::presentation_traits
                              <presentation_type> presentation_traits;
                            if(presentation_traits::is_occurring(first->second))
                            {
                              std::cerr << "should send key to presentation"
                                        << std::endl;
                              // boost::unwrap_ref(boost::unwrap_ref(first->second)
                              //                   .executor())
                              //   .key_process(key, pressed);
                              dirty = true;
                              processed_current_key_master = true;
                              break;
                            }
                          }
                        }
                        else
                        {
                          std::cerr << "not found current key master presentation"
                                    << std::endl;
                        }
                      }

                      if(!processed_current_key_master)
                      {
                        typedef structure_document_traits::media_lookupable
                          media_lookupable;
                        media_lookupable lookupable = structure_document_traits::media_lookup
                          (document);
                        typedef gntl::concept::lookupable_traits<media_lookupable> lookupable_traits;
                        typedef lookupable_traits::result_type lookup_result;
                        lookup_result r = lookupable_traits::lookup
                          (lookupable
                           , structure_document_traits::current_key_master(document));
                        dirty = true;
                      }
                    }
                  }

                  if(!processed_current_key_master)
                  {
                    std::cout << "evaluate select links" << std::endl;
                    gntl::algorithm::structure::context::evaluate_select_links
                      (gntl::ref(document.body)
                       , gntl::ref(document)
                       , key, screen_dimensions);
                    std::cout << "has focus: " << structure_document_traits::has_focus (document)
                              << " is bound: " << structure_document_traits::is_focus_bound(document)
                              << std::endl;
                    if(structure_document_traits::has_focus (document)
                       && structure_document_traits::is_focus_bound(document))
                    {
                      std::cout << "has focus and is bound" << std::endl;
                      typedef structure_document_traits::media_lookupable media_lookupable;
                      media_lookupable lookupable = structure_document_traits::media_lookup(document);
                      typedef gntl::concept::lookupable_traits<media_lookupable> lookupable_traits;
                      typedef lookupable_traits::result_type lookup_result;

                      lookup_result r = lookupable_traits::lookup
                        (lookupable
                         , structure_document_traits::focused_media(document));

                      if(r != lookupable_traits::not_found(lookupable))
                      {
                        std::cerr << "focused media " << key << std::endl;
                        gntl::algorithm::structure::media::focus_select
                          (*r, gntl::ref(document), key);
                        dirty = true;
                      }
                      else
                      {
                        std::cerr << "Couldn't find focused media" << std::endl;
                      }
                    }      
                  }
                }
              }
            }
            else
            {
              std::cerr << "Error reading key: " << r << std::endl;
            }
          }
        }

#ifndef GHTV_DISABLE_GSTREAMER
        if(!has_main_video)
        {
#endif
          dfb->WaitIdle(dfb.get());
#ifndef GHTV_DISABLE_GSTREAMER
        }
#endif
      }
    }
  }
  catch(std::exception& e)
  {
    std::cout << "Exception throw: " << typeid(e).name()
              << " what: " << e.what() << "\n"
#ifndef GNTL_NO_BOOST_EXCEPTION
              << boost::diagnostic_information(e) 
#endif
              << std::endl;
  }
}
