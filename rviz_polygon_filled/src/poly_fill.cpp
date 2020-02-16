#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include <OgreManualObject.h>
#include <OgreBillboardSet.h>

#include "rviz/display_context.h"
#include "rviz/frame_manager.h"
#include "rviz/properties/color_property.h"
#include "rviz/properties/float_property.h"
#include "rviz/properties/parse_color.h"
#include "rviz/validate_floats.h"

#include "poly_fill.h"

namespace rviz_polygon_filled
{

PolygonFilledDisplay::PolygonFilledDisplay()
{
  color_property_ = new rviz::ColorProperty( "Color", QColor( 25, 255, 0 ),
                                       "Color to draw the polygon.", this, SLOT( queueRender() ));
  alpha_property_ = new rviz::FloatProperty( "Alpha", 1.0,
                                       "Amount of transparency to apply to the polygon.", this, SLOT( queueRender() ));
  alpha_property_->setMin( 0 );
  alpha_property_->setMax( 1 );
}

PolygonFilledDisplay::~PolygonFilledDisplay()
{
  if ( initialized() )
  {
    scene_manager_->destroyManualObject( manual_object_ );
  }
}

void PolygonFilledDisplay::onInitialize()
{
  MFDClass::onInitialize();

  manual_object_ = scene_manager_->createManualObject();
  manual_object_->setDynamic( true );
  scene_node_->attachObject( manual_object_ );
}

void PolygonFilledDisplay::reset()
{
  MFDClass::reset();
  manual_object_->clear();
}

bool validateFloats( const geometry_msgs::PolygonStamped& msg )
{
  return rviz::validateFloats(msg.polygon.points);
}

void PolygonFilledDisplay::processMessage(const geometry_msgs::PolygonStamped::ConstPtr& msg)
{
  if( !validateFloats( *msg ))
  {
    setStatus( rviz::StatusProperty::Error, "Topic", "Message contained invalid floating point values (nans or infs)" );
    return;
  }

  Ogre::Vector3 position;
  Ogre::Quaternion orientation;
  if( !context_->getFrameManager()->getTransform( msg->header, position, orientation ))
  {
    ROS_DEBUG( "Error transforming from frame '%s' to frame '%s'",
               msg->header.frame_id.c_str(), qPrintable( fixed_frame_ ));
  }

  scene_node_->setPosition( position );
  scene_node_->setOrientation( orientation );

  manual_object_->clear();

  Ogre::ColourValue color = rviz::qtToOgre( color_property_->getColor() );
  color.a = alpha_property_->getFloat();
  // TODO: this does not actually support alpha as-is.  The
  // "BaseWhiteNoLighting" material ends up ignoring the alpha
  // component of the color values we set at each point.  Need to make
  // a material and do the whole setSceneBlending() rigamarole.

  uint32_t num_points = msg->polygon.points.size();
  if( num_points > 0 )
  {
    manual_object_->estimateVertexCount( num_points );
    manual_object_->begin( "BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP );
    for( uint32_t i=0; i < num_points + 1; ++i )
    {
      const geometry_msgs::Point32& msg_point = msg->polygon.points[ i % num_points ];
      manual_object_->position( msg_point.x, msg_point.y, msg_point.z );
      manual_object_->colour( color );
    }

    manual_object_->end();
  }
}

} // namespace rviz

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS( rviz_polygon_filled::PolygonFilledDisplay, rviz::Display )
