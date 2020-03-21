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

#include "poly2tri/poly2tri.h"

namespace rviz_polygon_filled
{

PolygonFilledDisplay::PolygonFilledDisplay()
{
  draw_fill_ = new rviz::BoolProperty("Fill",false, "Fill with color", this, SLOT( queueRender() ));
  draw_back_ = new rviz::BoolProperty("Backface",false, "Draw backface of polygon", this, SLOT( queueRender() ));
  color_fill_ = new rviz::ColorProperty( "Fill color", QColor( 25, 255, 0 ),
                                       "Color to fill the polygon.", this, SLOT( queueRender() ));

  draw_border_ = new rviz::BoolProperty("Border",false, "Draw border of polygon", this, SLOT( queueRender() ));
  color_border_ = new rviz::ColorProperty( "Border Color", QColor( 25, 255, 0 ),
                                       "Color to draw border of the polygon.", this, SLOT( queueRender() ));

  draw_mesh_ = new rviz::BoolProperty("Mesh",false, "Draw mesh of polygon", this, SLOT( queueRender() ));
  color_mesh_ = new rviz::ColorProperty( "Mesh Color", QColor( 25, 255, 0 ),
                                         "Color to draw mesh of the polygon.", this, SLOT( queueRender() ));
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

  std::vector<p2t::Point*> polyline;
  for (const auto &point : msg->polygon.points)
  {
    polyline.push_back(new p2t::Point(point.x, point.y));
  }

  p2t::CDT cdt{ polyline };

  cdt.Triangulate();
  const auto result = cdt.GetTriangles();

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

  Ogre::ColourValue colorFill = rviz::qtToOgre( color_fill_->getColor() );
  Ogre::ColourValue colorBorder = rviz::qtToOgre( color_border_->getColor() );
  Ogre::ColourValue colorMesh = rviz::qtToOgre( color_mesh_->getColor() );


  size_t num_points = result.size();
  if( num_points > 0 )
  {
    manual_object_->estimateVertexCount( num_points );

    if (draw_fill_->getBool())
    {
        manual_object_->begin( "BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST );
        for( size_t i = 0 ; i < result.size() ; ++i )
        {
          for(int j = 0 ; j < 3 ; ++j )
          {
              p2t::Point* point = result[i]->GetPoint(j);
              manual_object_->position( point->x, point->y, 0.0 );
              manual_object_->colour( colorFill );
          }

          if (draw_back_->getBool())
          {
              for(int j = 2 ; j >= 0 ; --j )
              {
                  p2t::Point* point = result[i]->GetPoint(j);
                  manual_object_->position( point->x, point->y, 0.0 );
                  manual_object_->colour( colorFill );
              }
          }
        }

        manual_object_->end();
    }


    if (draw_border_->getBool())
    {
        manual_object_->begin( "BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP );

        for (const auto &point : msg->polygon.points)
        {
          manual_object_->position( point.x, point.y, 0.0 );
          manual_object_->colour( colorBorder );
        }

        manual_object_->position( msg->polygon.points.front().x, msg->polygon.points.front().y, 0.0 );
        manual_object_->colour( colorBorder );
        manual_object_->end();
    }

    if (draw_mesh_->getBool())
    {
        manual_object_->begin( "BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST );


        for( size_t i = 0 ; i < result.size() ; ++i )
        {
          for(int j = 0 ; j < 3 ; ++j )
          {
              p2t::Point* pointA = result[i]->GetPoint(j);
              manual_object_->position( pointA->x, pointA->y, 0.0 );
              manual_object_->colour( colorMesh );

              p2t::Point* pointB = result[i]->GetPoint((j+1)%3);
              manual_object_->position( pointB->x, pointB->y, 0.0 );
              manual_object_->colour( colorMesh );

          }
        }
        manual_object_->end();
    }
  }
}

void PolygonFilledDisplay::drawPolygonBorder(const geometry_msgs::PolygonStamped::ConstPtr &msg)
{

}

} // namespace rviz

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS( rviz_polygon_filled::PolygonFilledDisplay, rviz::Display )
