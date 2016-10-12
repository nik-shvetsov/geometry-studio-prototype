#include "scenario.h"
#include "testtorus.h"

#include "gmlibsceneloader/gmlibsceneloaderdatadescription.h"

// openddl
#include "openddl/openddl.h"

// gmlib
#include <gmOpenglModule>
#include <gmSceneModule>
#include <gmParametricsModule>

// qt
#include <QTimerEvent>
#include <QDebug>

// stl
#include <cassert>
#include <iostream>
#include <iomanip>


using namespace std;

class SimStateLock
{
public:
    SimStateLock( Scenario& scenario ) : _scenario{scenario}
    {
        _state = _scenario.isSimulationRunning();
    }
    ~SimStateLock()
    {
        if(_state) _scenario.startSimulation();
        else       _scenario.stopSimulation();
    }

    Scenario&   _scenario;
    bool        _state;
};


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

  _select_renderer.reset();

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

  _select_renderer = std::make_shared<GMlib::DefaultSelectRenderer>();
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

    //_select_renderer = std::make_shared<GMlib::DefaultSelectRenderer>();

    _renderer->setCamera(_camera.get());
    _camera->set(init_cam_pos,init_cam_dir,init_cam_up);
    _camera->setCuttingPlanes( 1.0f, 8000.0f );
    _camera->rotateGlobal( GMlib::Angle(-45), GMlib::Vector<float,3>( 1.0f, 0.0f, 0.0f ) );
    _camera->translateGlobal( GMlib::Vector<float,3>( 0.0f, -20.0f, 20.0f ) );
    _camera->enableCulling(false); //
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

bool
Scenario::isSimulationRunning()
{
    return _scene->isRunning();
}

void
Scenario::replotTesttorus() { _testtorus->replot(4, 4, 1, 1); }

float Scenario::cameraSpeedScale()
{
    if( !_camera )
      return 1.0f;

    if(_camera->isLocked())
      return M_2PI * _camera->getLockDist();

    return _scene->getSphere().getRadius();
}

//void Scenario::lockObject(const bool &lockvar)
//{
//    if (lockvar==true)
//    {
//        _camera->lock(_testtorus.get());
//    }
//}

void Scenario::moveCamera(const QPoint& begin_pos, const QPoint& end_pos)
{
    auto pos = fromQtToGMlibViewPoint(*_camera.get(), begin_pos);
    auto prev = fromQtToGMlibViewPoint(*_camera.get(), end_pos);

    const float scale = cameraSpeedScale();
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
    _camera->move(GMlib::Vector<float,2> (wheel_delta * cameraSpeedScale() / _camera->getViewportH(), 0.0f));
}

void Scenario::panVerticalCam(int wheel_delta)
{
    _camera->move(GMlib::Vector<float,2>(0.0f, wheel_delta * cameraSpeedScale() / _camera->getViewportW()));
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
    _renderer->render();
}

void Scenario::camFly(GMlib::Vector<float,3> dS, GMlib::Angle dA, GMlib::Vector<float,3> axis)
{
    _camera->translateGlobal(dS);
    _camera->rotateGlobal(dA, axis);
}

void Scenario::camFlyUp()
{
    auto dS = GMlib::Vector<float,3>(0,0,0.1);
    auto dA = GMlib::Angle(0);
    auto axis = GMlib::Vector<float,3>(0,0,1);

    camFly(dS,dA,axis);
}

void Scenario::camFlyDown()
{
    auto dS = GMlib::Vector<float,3>(0,0,-0.1);
    auto dA = GMlib::Angle(0);
    auto axis = GMlib::Vector<float,3>(0,0,1);

    camFly(dS,dA,axis);
}

void Scenario::camFlyRight()
{
    auto dS = GMlib::Vector<float,3>(0.1,0,0);
    auto dA = GMlib::Angle(0);
    auto axis = GMlib::Vector<float,3>(0,0,1);

    camFly(dS,dA,axis);
}

void Scenario::camFlyLeft()
{
    auto dS = GMlib::Vector<float,3>(-0.1,0,0);
    auto dA = GMlib::Angle(0);
    auto axis = GMlib::Vector<float,3>(0,0,1);

    camFly(dS,dA,axis);
}

//


void Scenario::lockToObject(const QPoint& qpos)
{
    auto selected_obj = findSceneObj(qpos);
    if( selected_obj ) {_camera->lock( selected_obj );}
    else
        if(_camera->isLocked())
        {
            _camera->unLock();
        }

        else
        {
            _camera->lock( ( _scene->getSphereClean().getPos() - _camera->getPos() ) * _camera->getDir() );
        }
}

void Scenario::selectObject(const QPoint& qpos)
{
    auto selected_obj = findSceneObj(qpos);
    if( !selected_obj ) return;

    auto selected = selected_obj->isSelected(); //bool
    deselectAllObjects(); //for selecting only 1 object at a time
    selected_obj->setSelected( !selected );

    //_selectedObj = selected_obj;
}

void Scenario::selectObjects(const QPoint& qpos)
{
    auto obj = findSceneObj(qpos);
    if( obj ) obj->toggleSelected();
}

void Scenario::deselectAllObjects()
{
    //_selectedObj->setSelected(false);
    _scene->removeSelections();
}

void Scenario::selectSubObjects(GMlib::SceneObject* object)
{
    GMlib::Camera *cam   = dynamic_cast<GMlib::Camera*>( object );
    GMlib::Light  *light = dynamic_cast<GMlib::Light*>( object );
    if( !cam && !light ) {object->setSelected(true);}

    for( int i = 0; i < object->getChildren().getSize(); i++ )
    {
        selectSubObjects( (object->getChildren())[i] );
    }
}

void Scenario::selectAllObjects()
{
    deselectAllObjects();

    GMlib::Scene *scene = _scene.get();
    for( int i = 0; i < scene->getSize(); ++i )
    {
        selectSubObjects( (*scene)[i] );
    }
}

void Scenario::toogleSelectionAllObjects()
{
    if( _scene->getSelectedObjects().getSize() > 0 )
    {
        deselectAllObjects();
    }
    else
    {
        selectAllObjects();
    }
}

GMlib::SceneObject* Scenario::findSceneObj(const QPoint& qpos)
{
    GMlib::Point<int,2> pos = fromQtToGMlibViewPoint(*_camera.get(), qpos);
    GMlib::SceneObject* selected_obj = nullptr;

    const GMlib::Vector<int,2> size(_camera->getViewportW(), _camera->getViewportH());
    //const GMlib::Vector<int,2> size(QRect(0,0,200,200).width(), QRect(0,0,200,200).height());

    _select_renderer->setCamera(_camera.get());
    _select_renderer->reshape(size);
    _select_renderer->prepare();

    _select_renderer->select(GMlib::GM_SO_TYPE_SELECTOR); //+ selector only
    selected_obj =_select_renderer->findObject(pos(0),pos(1));

    if(!selected_obj)
    {
      _select_renderer->select( -GMlib::GM_SO_TYPE_SELECTOR );//- everything else
      selected_obj = _select_renderer->findObject(pos(0),pos(1));
    }

    _select_renderer->releaseCamera();

    return selected_obj;
}

void Scenario::moveObj(const QPoint& begin_pos, const QPoint& end_pos)
{
    auto pos = fromQtToGMlibViewPoint(*_camera.get(), begin_pos);
    auto prev = fromQtToGMlibViewPoint(*_camera.get(), end_pos);

    const float scale = cameraSpeedScale();
    auto deltaX = -(pos(0) - prev(0)) * scale / _camera->getViewportW();
    auto deltaY = -(pos(1) - prev(1))  *scale / _camera->getViewportH();

    GMlib::Vector<float,3> delta (deltaX,deltaY,0);

    const GMlib::Array<GMlib::SceneObject*> &selected_objects = _scene->getSelectedObjects();

    for( int i = 0; i < selected_objects.getSize(); i++ )
    {
        GMlib::SceneObject* obj = selected_objects(i);
        obj->translateGlobal(delta, true);
    }
}

void Scenario::rotateObj(const QPoint& begin_pos, const QPoint& end_pos)
{
    auto rot_pos = fromQtToGMlibViewPoint(*_camera.get(), begin_pos);
    auto rot_prev = fromQtToGMlibViewPoint(*_camera.get(), end_pos);

    auto difX = float(rot_pos(0) - rot_prev(0));
    auto difY = float(rot_pos(1) - rot_prev(1));

    GMlib::Vector<float,3> rotDir (difX,difY,0);

    rotDir = rotDir * 0.001;

    GMlib::Angle angle(M_2PI * sqrt(
                  pow( double( difX) / _camera->getViewportW(), 2 ) +
                  pow( double( difY) / _camera->getViewportH(), 2 ))
            );
    const GMlib::Array<GMlib::SceneObject*> &selected_objects = _scene->getSelectedObjects();
    for( int i = 0; i < selected_objects.getSize(); i++ )
    {
        GMlib::SceneObject* obj = selected_objects(i);
        obj->rotateGlobal(angle,rotDir);
    }

//    if(_selectedObj)
//    {
//        _selectedObj->rotateGlobal(angle,rotDir);
//    }
}

void Scenario::scaleObj(int &delta)
{
    const GMlib::Array<GMlib::SceneObject*> &selected_objects = _scene->getSelectedObjects();

    const float plus_val = 1.02;
    const float minus_val = 0.98;

    for( int i = 0; i < selected_objects.getSize(); i++ )
    {
        GMlib::SceneObject* obj = selected_objects(i);

        if(delta>0)
        {
            obj->scale( GMlib::Vector<float,3>( 0.1f + plus_val) );
        }
        else
        {
            obj->scale( GMlib::Vector<float,3>( 0.1f - minus_val) );
        }
    }
}

void Scenario::changeColor()
{
    const GMlib::Array<GMlib::SceneObject*> &selected_objects = _scene->getSelectedObjects();

    std::vector<GMlib::Material> colorsVec = {
        GMlib::GMmaterial::BlackPlastic,GMlib::GMmaterial::BlackRubber,
        GMlib::GMmaterial::Brass,GMlib::GMmaterial::Bronze,
        GMlib::GMmaterial::Chrome,GMlib::GMmaterial::Copper,
        GMlib::GMmaterial::Emerald,GMlib::GMmaterial::Gold,
        GMlib::GMmaterial::Jade,GMlib::GMmaterial::Obsidian,
        GMlib::GMmaterial::Pearl,GMlib::GMmaterial::Pewter,
        GMlib::GMmaterial::Plastic,GMlib::GMmaterial::PolishedBronze,
        GMlib::GMmaterial::PolishedCopper,GMlib::GMmaterial::PolishedGold,
        GMlib::GMmaterial::PolishedGreen,GMlib::GMmaterial::PolishedRed,
        GMlib::GMmaterial::PolishedSilver, GMlib::GMmaterial::Ruby,
        GMlib::GMmaterial::Sapphire, GMlib::GMmaterial::Silver,
        GMlib::GMmaterial::Snow, GMlib::GMmaterial::Turquoise
    };

    for( int i = 0; i < selected_objects.getSize(); i++ )
    {
        GMlib::SceneObject* obj = selected_objects(i);

        const auto objMat = obj->getMaterial();
        int colorNum = 0;

        for( unsigned int i = 0; i < colorsVec.size(); ++i )
        {
            if (objMat == colorsVec[i])
            {
                colorNum = i;
                break;
            }
        }
        colorNum++;
        if(colorNum < 22) //colors.size()-2
        {
            obj->setMaterial(colorsVec[colorNum]);
        }
        else
        {
            obj->setMaterial(colorsVec[0]);
        }
    }
}

// Makes a point for camera movement
GMlib::Point<int, 2> Scenario::fromQtToGMlibViewPoint(const GMlib::Camera& cam, const QPoint& pos)
{
    int h = cam.getViewportH(); // Height of the camera’s viewport

    // QPoint
    int qPointX = pos.x();
    int qPointY = pos.y();

    // GMlib Point
    int pointX = qPointX;
    int pointY = h - qPointY - 1;

    return GMlib::Point<int, 2> (pointX, pointY);
}

void Scenario::load() {

  qDebug() << "Open scene...";
  //SimStateLock a(*this);
  stopSimulation();

  auto filename = std::string("gmlib_save.openddl");
  auto is = std::ifstream(filename,std::ios_base::in);

  if(!is.is_open())
  {
    std::cerr << "Unable to open " << filename << " for reading..."
              << std::endl;
    return;
  }

  is.seekg( 0, std::ios_base::end );
  auto buff_length = is.tellg();
  is.seekg( 0, std::ios_base::beg );

  std::vector<char> buffer(buff_length);
  is.read(buffer.data(),buff_length);


  std::cout << "Buffer length: " << buff_length << std::endl;

  GMlibSceneLoaderDataDescription gsdd;

  ODDL::DataResult result = gsdd.ProcessText(buffer.data());
  if(result != ODDL::kDataOkay) {

    auto res_to_char = [](auto nr, const ODDL::DataResult& result) {
      return char(((0xff << (8*nr)) & result ) >> (8*nr));
    };

    auto res_to_str = [&res_to_char](const ODDL::DataResult& result) {
      return std::string() + res_to_char(3,result) + res_to_char(2,result) + res_to_char(1,result) + res_to_char(0,result);
    };

    std::cerr << "Data result no A-OK: " << res_to_str(result) << " (" << result << ")" << std::endl;
    return;
  }

  std::cout << "Data result A-OK" << std::endl;
  auto structure = gsdd.GetRootStructure()->GetFirstSubnode();
  while(structure) {

    // Do something ^^,
    // Travers the ODDL structures and build your scene objects

    structure = structure->Next();
  }

  // Load GMlib::SceneObjects into the scene.

}

void
Scenario::save() {

  qDebug() << "Saving scene...";
  //SimStateLock a(*this);
  stopSimulation();

  auto filename = std::string("gmlib_save.openddl");
  auto os = std::ofstream(filename,std::ios_base::out);
  if(!os.is_open())
  {
    std::cerr << "Unable to open " << filename << " for saving..."
              << std::endl;
    return;
  }


  os << "GMlibVersion { int32 { 0x"
     << std::setw(6) << std::setfill('0')
     << std::hex << GM_VERSION
     << " } }"
     << std::endl;


  auto &scene = *_scene;
  for( auto i = 0; i < scene.getSize(); ++i )
  {
    const auto obj = scene[i];
    save(os,obj);
  }

  startSimulation();
  qDebug() << "The scene was success saved. Starting simulation.";

}

void Scenario::save(std::ofstream &os, const GMlib::SceneObject *obj)
{
    auto cam_obj = dynamic_cast<const GMlib::Camera*>(obj);
    if(cam_obj) return;

    os << obj->getIdentity() << std::endl
           << "{" << std::endl<<std::endl;

    saveSO(os,obj);

    auto torus = dynamic_cast<const GMlib::PTorus<float>*>(obj);
    if(torus)
    {
        savePT(os,torus);
    }

    auto sphere = dynamic_cast<const GMlib::PSphere<float>*>(obj);
    if(sphere)
    {
        savePS(os,sphere);
    }

    auto  cylinder = dynamic_cast<const GMlib::PCylinder<float>*>(obj);
    if(cylinder)
    {
        savePC(os,cylinder);
    }

    auto  plane = dynamic_cast<const GMlib::PPlane<float>*>(obj);
    if(plane)
    {
        savePP(os,plane);
    }

    const auto& children = obj->getChildren();

    for(auto i = 0; i < children.getSize(); ++i)
    {
        save(os,children(i));
    }
    os << "}" << std::endl;
}

void Scenario::saveSO(std::ofstream &os, const GMlib::SceneObject *obj)
{
    //os << "SceneObjectData" << std::endl << "{" << std::endl;
    //os << "setCollapsed( bool {" << "  " << ( obj->isCollapsed()?"true":"false") << "} )";
    //os << "}" << std::endl;



    os << "SceneObjectData" << endl
       << "{" << endl<<endl;


    os << "set"<<endl<<"{"<<endl<<"Point {"
       << " float[3] { " << obj->getPos()(0)<<", "<<obj->getPos()(1)<<", "<<obj->getPos()(2)<<" }"
       << " }"<<endl;
    os <<"Vector {"
      << " float[3] { " << obj->getDir()(0)<<", "<<obj->getDir()(1)<<", "<<obj->getDir()(2)<<" }"
      << " }"<<endl;
    os <<"Vector {"
      << " float[3] { " << obj->getUp()(0)<<", "<<obj->getUp()(1)<<", "<<obj->getUp()(2)<<" }"
      << " }"<<endl<<"}"<<endl<<endl;


    os << "setCollapsed{ bool {"
       << ( obj->isCollapsed()?"true":"false")
       << "} }"<<endl<<endl;
    os << "setLighted{ bool {"
       << ( obj->isLighted()?"true":"false")
       << "} }"<<endl<<endl;
    os << "setVisible{ bool {"
       << ( obj->isVisible()?"true":"false")
       << "} }"<<endl<<endl;


    os << "setColor"<<endl<<"{"<<endl<<"Color {"
       << " double[3] { " << obj->getColor().getRedC()<<", "<<obj->getColor().getGreenC()<<", "<<obj->getColor().getBlueC()<<" }"
       << " }"<<endl<<"}"<<endl<<endl;


    os << "setMaterial"<<endl<<"{"<<endl<<"Material"<<endl<<"{"<<endl<<"Color {"
       << " double[3] { " << obj->getMaterial().getAmb().getRedC()<<", "<<obj->getMaterial().getAmb().getGreenC()<<", "<<obj->getMaterial().getAmb().getBlueC()<<" }"
       << " }"<<endl;
    os <<"Color {"
      << " double[3] { " << obj->getMaterial().getDif().getRedC()<<", "<<obj->getMaterial().getDif().getGreenC()<<", "<<obj->getMaterial().getDif().getBlueC()<<" }"
      << " }"<<endl;
    os <<"Color {"
      << " double[3] { " << obj->getMaterial().getSpc().getRedC()<<", "<<obj->getMaterial().getSpc().getGreenC()<<", "<<obj->getMaterial().getSpc().getBlueC()<<" }"
      << " }"<<endl;
    os<< "float {" <<obj->getMaterial().getShininess()  << "}"<<endl
      << "}"<<endl<<"}"<<endl;
    os << "}" << endl<<endl;


}

void Scenario::savePT(std::ofstream &os, const GMlib::PTorus<float> *obj)
{
//    os << "PTorusData" << std::endl << "{" << std::endl;
//    os << "}" << std::endl;

    os << "PSurfData"<<endl<<"{"<<endl
       <<"enableDefaultVisualize { bool {" << ( obj->getVisualizers()(0)?"true":"false")<<"} "
      << " }"<<endl<<endl;
    os <<"replot {"<<endl
      << "int {" <<obj->getSamplesU()<<"}"<<endl
      << "int {" <<obj->getSamplesV()<<"}"<<endl
      << "int {" <<obj->getDerivativesU()<<"}"<<endl
      << "int {" <<obj->getDerivativesV()<<"}"<<endl;
    os << "}" <<endl<<"}"<<endl<<endl;


    os << "PTorusData" << std::endl
       << "{" << endl;

    os << "setTubeRadius1{ float {"<<  obj->getTubeRadius1()<< "} }"<<endl;
    os << "setTubeRadius2{ float {"<<  obj->getTubeRadius2()<< "} }"<<endl;
    os << "setWheelRadius{ float {"<<  obj->getWheelRadius()<< "} }"<<endl;

    os << "}" <<endl<<endl;
}

void Scenario::savePS(std::ofstream &os, const GMlib::PSphere<float> *obj) {

    using namespace std;

    os << "PSurfData"<<endl<<"{"<<endl
       <<"enableDefaultVisualize { bool {" << ( obj->getVisualizers()(0)?"true":"false")<<"} "
      << " }"<<endl<<endl;
    os <<"replot {"<<endl
      << "int {" <<obj->getSamplesU()<<"}"<<endl
      << "int {" <<obj->getSamplesV()<<"}"<<endl
      << "int {" <<obj->getDerivativesU()<<"}"<<endl
      << "int {" <<obj->getDerivativesV()<<"}"<<endl;
    os << "}" <<endl<<"}"<<endl<<endl;


    os << "PSphereData" << std::endl
       << "{" << endl;

    os << "setRadius{ float {"<<  obj->getRadius()<< "} }"<<endl;

    os << "}" <<endl<<endl;
}

void Scenario::savePC(std::ofstream &os, const GMlib::PCylinder<float> *obj) {

    os << "PSurfData"<<endl<<"{"<<endl
       <<"enableDefaultVisualize { bool {" << ( obj->getVisualizers()(0)?"true":"false")<<"} "
      << " }"<<endl<<endl;

    os <<"replot {"<<endl
      << "int {" <<obj->getSamplesU()<<"}"<<endl
      << "int {" <<obj->getSamplesV()<<"}"<<endl
      << "int {" <<obj->getDerivativesU()<<"}"<<endl
      << "int {" <<obj->getDerivativesV()<<"}"<<endl;

    os << "}" <<endl<<"}"<<endl<<endl;

    os << "PCylinderData" << std::endl
       << "{" << endl;

    os <<"setConstants {"<<endl
      << "float {" <<obj->getRadiusX()<<"}"<<endl
      << "float {" <<obj->getRadiusY()<<"}"<<endl
      << "float {" <<obj->getHeight()<<"}"<<endl;

    os << "}" <<endl<<"}"<<endl<<endl;

    os << "}" <<endl<<endl;
}

void Scenario::savePP(std::ofstream &os, const GMlib::PPlane<float> *obj) {

    os << "PSurfData"<<endl<<"{"<<endl
       <<"enableDefaultVisualize { bool {" << ( obj->getVisualizers()(0)?"true":"false")<<"} "
      << " }"<<endl<<endl;

    os <<"replot {"<<endl
      << "int {" <<obj->getSamplesU()<<"}"<<endl
      << "int {" <<obj->getSamplesV()<<"}"<<endl
      << "int {" <<obj->getDerivativesU()<<"}"<<endl
      << "int {" <<obj->getDerivativesV()<<"}"<<endl;

    os << "}" <<endl<<"}"<<endl<<endl;


    os << "PPlaneData" << std::endl
       << "{" << endl;

    //os << "setRadius{ float {"<<  obj->getRadius()<< "} }"<<endl;

    os << "}" <<endl<<endl;
}



