#include "serialize.hpp"

#include <osgDB/ObjectWrapper>
#include <osgDB/Registry>

#include <components/nifosg/fog.hpp>
#include <components/nifosg/matrixtransform.hpp>

#include <components/sceneutil/morphgeometry.hpp>
#include <components/sceneutil/positionattitudetransform.hpp>
#include <components/sceneutil/riggeometry.hpp>
#include <components/sceneutil/riggeometryosgaextension.hpp>
#include <components/sceneutil/skeleton.hpp>
#include <components/sceneutil/texturetype.hpp>

namespace SceneUtil
{

    template <class Cls>
    static osg::Object* createInstanceFunc()
    {
        return new Cls;
    }

    class PositionAttitudeTransformSerializer : public osgDB::ObjectWrapper
    {
    public:
        PositionAttitudeTransformSerializer()
            : osgDB::ObjectWrapper(createInstanceFunc<SceneUtil::PositionAttitudeTransform>,
                "SceneUtil::PositionAttitudeTransform",
                "osg::Object osg::Node osg::Group osg::Transform SceneUtil::PositionAttitudeTransform")
        {
            addSerializer(new osgDB::PropByRefSerializer<SceneUtil::PositionAttitudeTransform, osg::Vec3f>("position",
                              osg::Vec3f(), &SceneUtil::PositionAttitudeTransform::getPosition,
                              &SceneUtil::PositionAttitudeTransform::setPosition),
                osgDB::BaseSerializer::RW_VEC3F);
            addSerializer(new osgDB::PropByRefSerializer<SceneUtil::PositionAttitudeTransform, osg::Quat>("attitude",
                              osg::Quat(), &SceneUtil::PositionAttitudeTransform::getAttitude,
                              &SceneUtil::PositionAttitudeTransform::setAttitude),
                osgDB::BaseSerializer::RW_QUAT);
            addSerializer(
                new osgDB::PropByRefSerializer<SceneUtil::PositionAttitudeTransform, osg::Vec3f>("scale", osg::Vec3f(),
                    &SceneUtil::PositionAttitudeTransform::getScale, &SceneUtil::PositionAttitudeTransform::setScale),
                osgDB::BaseSerializer::RW_VEC3F);
        }
    };

    class SkeletonSerializer : public osgDB::ObjectWrapper
    {
    public:
        SkeletonSerializer()
            : osgDB::ObjectWrapper(createInstanceFunc<SceneUtil::Skeleton>, "SceneUtil::Skeleton",
                "osg::Object osg::Node osg::Group SceneUtil::Skeleton")
        {
        }
    };

    class RigGeometrySerializer : public osgDB::ObjectWrapper
    {
    public:
        RigGeometrySerializer()
            : osgDB::ObjectWrapper(createInstanceFunc<SceneUtil::RigGeometry>, "SceneUtil::RigGeometry",
                "osg::Object osg::Node osg::Drawable SceneUtil::RigGeometry")
        {
        }
    };

    class RigGeometryHolderSerializer : public osgDB::ObjectWrapper
    {
    public:
        RigGeometryHolderSerializer()
            : osgDB::ObjectWrapper(createInstanceFunc<SceneUtil::RigGeometryHolder>, "SceneUtil::RigGeometryHolder",
                "osg::Object osg::Node osg::Drawable SceneUtil::RigGeometryHolder")
        {
        }
    };

    class OsgaRigGeometrySerializer : public osgDB::ObjectWrapper
    {
    public:
        OsgaRigGeometrySerializer()
            : osgDB::ObjectWrapper(createInstanceFunc<SceneUtil::OsgaRigGeometry>, "SceneUtil::OsgaRigGeometry",
                "osg::Object osg::Node osg::Geometry osgAnimation::RigGeometry SceneUtil::OsgaRigGeometry")
        {
        }
    };

    class MorphGeometrySerializer : public osgDB::ObjectWrapper
    {
    public:
        MorphGeometrySerializer()
            : osgDB::ObjectWrapper(createInstanceFunc<SceneUtil::MorphGeometry>, "SceneUtil::MorphGeometry",
                "osg::Object osg::Node osg::Drawable SceneUtil::MorphGeometry")
        {
        }
    };

    class LightManagerSerializer : public osgDB::ObjectWrapper
    {
    public:
        LightManagerSerializer()
            : osgDB::ObjectWrapper(createInstanceFunc<osg::Group>, "SceneUtil::LightManager",
                "osg::Object osg::Node osg::Group SceneUtil::LightManager")
        {
        }
    };

    class CameraRelativeTransformSerializer : public osgDB::ObjectWrapper
    {
    public:
        CameraRelativeTransformSerializer()
            : osgDB::ObjectWrapper(createInstanceFunc<osg::Group>, "MWRender::CameraRelativeTransform",
                "osg::Object osg::Node osg::Group MWRender::CameraRelativeTransform")
        {
        }
    };

    class MatrixTransformSerializer : public osgDB::ObjectWrapper
    {
    public:
        MatrixTransformSerializer()
            : osgDB::ObjectWrapper(createInstanceFunc<NifOsg::MatrixTransform>, "NifOsg::MatrixTransform",
                "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform NifOsg::MatrixTransform")
        {
        }
    };

    class FogSerializer : public osgDB::ObjectWrapper
    {
    public:
        FogSerializer()
            : osgDB::ObjectWrapper(
                createInstanceFunc<osg::Fog>, "NifOsg::Fog", "osg::Object osg::StateAttribute osg::Fog NifOsg::Fog")
        {
            addSerializer(new osgDB::PropByValSerializer<NifOsg::Fog, float>(
                              "Depth", 1.f, &NifOsg::Fog::getDepth, &NifOsg::Fog::setDepth),
                osgDB::BaseSerializer::RW_FLOAT);
        }
    };

    class TextureTypeSerializer : public osgDB::ObjectWrapper
    {
    public:
        TextureTypeSerializer()
            : osgDB::ObjectWrapper(createInstanceFunc<SceneUtil::TextureType>, "SceneUtil::TextureType",
                "osg::Object osg::StateAttribute SceneUtil::TextureType")
        {
        }
    };

    osgDB::ObjectWrapper* makeDummySerializer(const std::string& classname)
    {
        return new osgDB::ObjectWrapper(createInstanceFunc<osg::DummyObject>, classname, "osg::Object");
    }

    class GeometrySerializer : public osgDB::ObjectWrapper
    {
    public:
        GeometrySerializer()
            : osgDB::ObjectWrapper(
                createInstanceFunc<osg::Drawable>, "osg::Geometry", "osg::Object osg::Drawable osg::Geometry")
        {
        }
    };

    void registerSerializers()
    {
        static bool done = false;
        if (!done)
        {
            osgDB::ObjectWrapperManager* mgr = osgDB::Registry::instance()->getObjectWrapperManager();
            mgr->addWrapper(new PositionAttitudeTransformSerializer);
            mgr->addWrapper(new SkeletonSerializer);
            mgr->addWrapper(new RigGeometrySerializer);
            mgr->addWrapper(new RigGeometryHolderSerializer);
            mgr->addWrapper(new OsgaRigGeometrySerializer);
            mgr->addWrapper(new MorphGeometrySerializer);
            mgr->addWrapper(new LightManagerSerializer);
            mgr->addWrapper(new CameraRelativeTransformSerializer);
            mgr->addWrapper(new MatrixTransformSerializer);
            mgr->addWrapper(new FogSerializer);
            mgr->addWrapper(new TextureTypeSerializer);

            // Don't serialize Geometry data as we are more interested in the overall structure rather than tons of
            // vertex data that would make the file large and hard to read.
            mgr->removeWrapper(mgr->findWrapper("osg::Geometry"));
            mgr->addWrapper(new GeometrySerializer);

            // ignore the below for now to avoid warning spam
            const char* ignore[] = {
                "Debug::DebugDrawer",
                "MWRender::NifAnimBlendController",
                "MWRender::BoneAnimBlendController",
                "MWRender::BoneAnimBlendControllerWrapper",
                "MWRender::PtrHolder",
                "Resource::TemplateRef",
                "Resource::TemplateMultiRef",
                "SceneUtil::CompositeStateSetUpdater",
                "SceneUtil::UBOManager",
                "SceneUtil::LightListCallback",
                "SceneUtil::LightManagerUpdateCallback",
                "SceneUtil::FFPLightStateAttribute",
                "SceneUtil::UpdateRigBounds",
                "SceneUtil::UpdateRigGeometry",
                "SceneUtil::LightSource",
                "SceneUtil::DisableLight",
                "SceneUtil::MWShadowTechnique",
                "SceneUtil::TextKeyMapHolder",
                "Shader::AddedState",
                "Shader::RemovedAlphaFunc",
                "NifOsg::BillboardCallback",
                "NifOsg::FlipController",
                "NifOsg::KeyframeController",
                "NifOsg::Emitter",
                "NifOsg::ParticleColorAffector",
                "NifOsg::ParticleSystem",
                "NifOsg::GravityAffector",
                "NifOsg::ParticleBomb",
                "NifOsg::GrowFadeAffector",
                "NifOsg::InverseWorldMatrix",
                "NifOsg::StaticBoundingBoxCallback",
                "NifOsg::GeomMorpherController",
                "NifOsg::UpdateMorphGeometry",
                "NifOsg::UVController",
                "NifOsg::VisController",
                "osgMyGUI::Drawable",
                "osg::DrawCallback",
                "osg::UniformBufferObject",
                "osgOQ::ClearQueriesCallback",
                "osgOQ::RetrieveQueriesCallback",
                "osg::DummyObject",
            };
            for (size_t i = 0; i < sizeof(ignore) / sizeof(ignore[0]); ++i)
            {
                mgr->addWrapper(makeDummySerializer(ignore[i]));
            }

            done = true;
        }
    }

}
