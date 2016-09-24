#include "scenario.h"
#include "testtorus.h"

// gmlib
#include <gmOpenglModule>
#include <gmSceneModule>
#include <gmParametricsModule>

// qt
#include <QTimerEvent>
#include <QDebug>

// stl
#include <cassert>

Scenario::Scenario() : QObject(), _timer_id{0}/*, _select_renderer{nullptr}*/ {

  if(_instance != nullptr) {

    std::cerr << "This version of the Scenario only supports a single instance of the GMlibWraper..." << std::endl;
    std::cerr << "Only one of me(0x" << this << ") please!! ^^" << std::endl;
    assert(!_instance);
    exit(666);
  }

  _instance = std::unique_ptr<Scenario>(this);
}

Scenario::~Scenario() {

  _instance.release();
}

void
Scenario::deinitialize() {

  stopSimulation();

  _scene->remove(_testtorus.get());
  _testtorus.reset();

  _scene->removeLight(_light.get());
  _light.reset();

  _renderer->releaseCamera();
  _scene->removeCamera(_camera.get());

  _renderer.reset();
  _camera.reset();

  _scene->clear();
  _scene.reset();

  // Clean up GMlib GL backend
  GMlib::GL::OpenGLManager::cleanUp();
}

void
Scenario::initialize() {

  // Setup and initialized GMlib GL backend
  GMlib::GL::OpenGLManager::init();

  // Setup and init the GMlib GMWindow
  _scene = std::make_shared<GMlib::Scene>();
}

void
Scenario::initializeScenario() {

  // Insert a light
  auto init_light_pos = GMlib::Point<GLfloat,3>( 2.0, 4.0, 10 );
  _light = std::make_shared<GMlib::PointLight>( GMlib::GMcolor::White, GMlib::GMcolor::White,
                                                GMlib::GMcolor::White, init_light_pos );
  _light->setAttenuation(0.8, 0.002, 0.0008);
  _scene->insertLight( _light.get(), false );

  // Insert Sun
  _scene->insertSun();

  // Default camera parameters
  auto init_viewport_size = 600;
  auto init_cam_pos       = GMlib::Point<float,3>(  0.0f, 0.0f, 0.0f );
  auto init_cam_dir       = GMlib::Vector<float,3>( 0.0f, 1.0f, 0.0f );
  auto init_cam_up        = GMlib::Vector<float,3>(  0.0f, 0.0f, 1.0f );

    // Projection cam 0
    _camera = std::make_shared<GMlib::Camera>();
    _renderer = std::make_shared<GMlib::DefaultRenderer>();

    _renderer->setCamera(_camera.get());
    _camera->set(init_cam_pos,init_cam_dir,init_cam_up);
    _camera->setCuttingPlanes( 1.0f, 8000.0f );
    _camera->rotateGlobal( GMlib::Angle(-45), GMlib::Vector<float,3>( 1.0f, 0.0f, 0.0f ) );
    _camera->translateGlobal( GMlib::Vector<float,3>( 0.0f, -20.0f, 20.0f ) );
    _scene->insertCamera( _camera.get() );
    _renderer->reshape( GMlib::Vector<int,2>(init_viewport_size, init_viewport_size) );


//    // Front cam 1
//    auto rendererFront = std::make_shared<GMlib::DefaultRenderer>();
//    auto cameraFront = std::make_shared<GMlib::Camera>();

//    rendererFront->setCamera(cameraFront.get());
//    cameraFront->set( init_cam_pos + GMlib::Vector<float,3>( 0.0f, -50.0f, 0.0f ), init_cam_dir, init_cam_up );
//    cameraFront->setCuttingPlanes( 1.0f, 8000.0f );
//    _scene->insertCamera( cameraFront.get() );
//    rendererFront->reshape( GMlib::Vector<int,2>(init_viewport_size, init_viewport_size) );
//    _camerasVec.emplace_back(cameraFront);
//    _rendererVec.emplace_back(rendererFront);

//    // Side cam 2
//    auto rendererSide = std::make_shared<GMlib::DefaultRenderer>();
//    auto cameraSide = std::make_shared<GMlib::Camera>();

//    rendererSide->setCamera(cameraSide.get());
//    cameraSide->set( init_cam_pos + GMlib::Vector<float,3>( -50.0f, 0.0f, 0.0f ), GMlib::Vector<float,3>( 1.0f, 0.0f, 0.0f ), init_cam_up );
//    cameraSide->setCuttingPlanes( 1.0f, 8000.0f );
//    _scene->insertCamera( cameraSide.get() );
//    rendererSide->reshape( GMlib::Vector<int,2>(init_viewport_size, init_viewport_size) );
//    _camerasVec.emplace_back(cameraSide);
//    _rendererVec.emplace_back(rendererSide);

//    // Top cam 3
//    auto rendererTop = std::make_shared<GMlib::DefaultRenderer>();
//    auto cameraTop = std::make_shared<GMlib::Camera>();

//    rendererTop->setCamera(cameraTop.get());
//    cameraTop->set( init_cam_pos + GMlib::Vector<float,3>( 0.0f, 0.0f, 50.0f ), -init_cam_up, init_cam_dir );
//    cameraTop->setCuttingPlanes( 1.0f, 8000.0f );
//    _scene->insertCamera( cameraTop.get() );
//    rendererTop->reshape( GMlib::Vector<int,2>(init_viewport_size, init_viewport_size) );
//    _camerasVec.emplace_back(cameraTop);
//    _rendererVec.emplace_back(rendererTop);


//    //initializing
//    _camera = _camerasVec.front();
//    _renderer = _rendererVec.front();



  // Surface
  _testtorus = std::make_shared<TestTorus>();
  _testtorus->toggleDefaultVisualizer();
  _testtorus->replot(200,200,1,1);
  _scene->insert(_testtorus.get());

  _testtorus->test01();

  auto plane = new GMlib::PPlane<float> (GMlib::Point<float,3>(-5,0,0), GMlib::Vector<float,3>(10,0,0), GMlib::Vector<float,3>(0,20,0));
  plane->toggleDefaultVisualizer();
  plane->replot(50,50,1,1);
  _scene->insert(plane);

  //----------------------
}

std::unique_ptr<Scenario> Scenario::_instance {nullptr};

Scenario&
Scenario::instance() { return *_instance; }

void
Scenario::prepare() { _scene->prepare(); }

void
Scenario::render( const QRect& viewport_in, GMlib::RenderTarget& target ) {

  // Update viewport
  if(_viewport != viewport_in) {

    _viewport = viewport_in;

    const auto& size = _viewport.size();
    _renderer->reshape( GMlib::Vector<int,2>(size.width(),size.height()));
    _camera->reshape( 0, 0, size.width(), size.height() );
  }

  // Render and swap buffers
  _renderer->render(target);
}

void
Scenario::startSimulation() {

  if( _timer_id || _scene->isRunning() )
    return;

  _timer_id = startTimer(16, Qt::PreciseTimer);
  _scene->start();
}

void
Scenario::stopSimulation() {

  if( !_timer_id || !_scene->isRunning() )
    return;

  _scene->stop();
  killTimer(_timer_id);
  _timer_id = 0;
}

void
Scenario::timerEvent(QTimerEvent* e) {

  e->accept();

  _scene->simulate();
  prepare();
}

void
Scenario::toggleSimulation() { _scene->toggleRun(); }

void
Scenario::replotTesttorus() { _testtorus->replot(4, 4, 1, 1); }

float Scenario::cameraSpeedScale(const bool &lockvar)
{
   if (lockvar==true)
   {
       _camera->lock(_testtorus.get());

       return M_2PI * _camera->getLockDist();
   }

  return _scene->getSphere().getRadius();
}

void Scenario::lockObject(const bool &lockvar){
    if (lockvar==true)
    {
        _camera->lock(_testtorus.get());
    }
}

void Scenario::moveCamera(const QPoint& begin_pos, const QPoint& end_pos)
{
    auto pos = fromQtToGMlibViewPoint(*_camera.get(), begin_pos);
    auto prev = fromQtToGMlibViewPoint(*_camera.get(), end_pos);

    const float scale = cameraSpeedScale(false);
    GMlib::Vector<float,2> delta ( -(pos(0) - prev(0))*scale / _camera->getViewportW(),
                                    (pos(1) - prev(1))*scale / _camera->getViewportH()      );

    //delta*=0.01;
    _camera->move( delta );
}

void Scenario::zoomCamera(const float &zoom_val)
{
    _camera->zoom(zoom_val);
}

void Scenario::panHorizontalCam(int wheel_delta)
{
    _camera->move(GMlib::Vector<float,2> (wheel_delta * cameraSpeedScale(false) / _camera->getViewportH(), 0.0f));
}

void Scenario::panVerticalCam(int wheel_delta)
{
    _camera->move(GMlib::Vector<float,2>(0.0f, wheel_delta * cameraSpeedScale(false) / _camera->getViewportW()));
}

void Scenario::switchCam(int n)
{
    //auto init_viewport_size = 600;
    auto init_cam_pos       = GMlib::Point<float,3>(  0.0f, 0.0f, 0.0f );
    auto init_cam_dir       = GMlib::Vector<float,3>( 0.0f, 1.0f, 0.0f );
    auto init_cam_up        = GMlib::Vector<float,3>(  0.0f, 0.0f, 1.0f );
    switch (n)
    {
    case 1:
        _camera->set(init_cam_pos,init_cam_dir,init_cam_up);
        _camera->rotateGlobal( GMlib::Angle(-45), GMlib::Vector<float,3>( 1.0f, 0.0f, 0.0f ) );
        _camera->translateGlobal( GMlib::Vector<float,3>( 0.0f, -20.0f, 20.0f ) );
        qDebug() << "Projection cam";
        break;
    case 2:
        _camera->set(init_cam_pos,init_cam_dir,init_cam_up);
        _camera->rotateGlobal( GMlib::Angle(0), GMlib::Vector<float,3>( 1.0f, 0.0f, 0.0f ) );
        _camera->translateGlobal( GMlib::Vector<float,3>( 0.0f, -20.0f, 0.0f ) );
        qDebug() << "Front cam";
        break;
    case 3:
        _camera->set(init_cam_pos,init_cam_dir,init_cam_up);
        _camera->rotateGlobal( GMlib::Angle(90), GMlib::Vector<float,3>( 0.0f, 1.0f, 0.0f ) );
        _camera->translateGlobal( GMlib::Vector<float,3>( 0.0f, -20.0f, 0.0f ) );
        qDebug() << "Side cam";
        break;
    case 4:
        _camera->set(init_cam_pos,init_cam_dir,init_cam_up);
        _camera->rotateGlobal( GMlib::Angle(-90), GMlib::Vector<float,3>( 1.0f, 0.0f, 0.0f ) );
        _camera->translateGlobal( GMlib::Vector<float,3>( 0.0f, 0.0f, 20.0f ) );
        qDebug() << "Top cam";
        break;
    }
}

void Scenario::lockToObject()
{
//    auto view_name = viewNameFromParams(params);
//    auto pos       = toGMlibViewPoint(view_name, posFromParams(params));

//    auto cam     = findCamera(view_name);
//    auto sel_obj = findSceneObject(view_name,pos);

//    if( sel_obj )
//      _camera->lock( sel_obj );

//    else if(_camera->isLocked()) _camera->unLock();
//            else
//            {
//                _camera->lock( ( _scene->getSphereClean().getPos() - _camera->getPos() ) * _camera->getDir() );
//            }
}

void Scenario::selectObject()
{
//    auto view_name = viewNameFromParams(params);
//    auto pos       = toGMlibViewPoint(view_name, posFromParams(params));

//    auto obj = findSceneObject(view_name,pos);
//    if( !obj )
//      return;

//    // Preserver object selection
//    auto selected = obj->isSelected();
//    heDeSelectAllObjects();
//    obj->setSelected( !selected );
}

// Makes a point for camera movement
GMlib::Point<int, 2> Scenario::fromQtToGMlibViewPoint(const GMlib::Camera& cam, const QPoint& pos)
{
    int h = cam.getViewportH(); // Height of the camera’s viewport
    // QPoint
    int q1 {pos.x()};
    int q2 {pos.y()};
    // GMlib Point
    int p1 = q1;
    int p2 = h - q2 - 1;
    return GMlib::Point<int, 2> {p1, p2};
}

