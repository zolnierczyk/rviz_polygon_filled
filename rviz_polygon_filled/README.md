# rviz_polygon_filled

![CI](https://github.com/zolnierczyk/rviz_polygon_filled/workflows/CI/badge.svg)

Plugin for rviz which display filled polygon.

# Examples

## Be aware! Work in Progress

This plugin does not work! Yet :)

## How filled polygons work?


## Where are filled polygons?

ROS with its rviz 3D visualization environment is very very powerfull tool. In project on which I was working we were
visualizing multiple data in realtime to provide situational awarness for our stuff. I must also said that with
litle more work we cloud achive not only usable but also beautifull visualization (of course for enginers!).

Some day we got job to visualize filled polygon. Should be easy task! And I found that ... it is not easy at all.
By default rviz do not support such task. It can only display polygon as border. Ok but there should be such plugins
made by _community_ which just do it. There is very nice library called [jsk_visualization](https://github.com/jsk-ros-pkg/jsk_visualization)
and it has custom plugin for rviz which can display custom visualization message called PolygonArray. Sadly this plugin
does not fullfill our needs:

* properties of visualization like color and alpha was defined inside message not as properties in rviz
** yep this topic is about principle who should define colors on rviz
* visualization was bugged when polygon was concave
* visualization does not handle case when polygon has holes inside

As result in decision was done in project to workaround problem.

But this task stay with me for some time and finally. I decided to make such plugin which will do it right.

## Mathematics

Why filled polygon was hard task to be done? This is because of way how computer draw objects on monitor. In general
graphic card is working on triangles. Triangle is defined by three points which order is crucial. Those three points
define plane from which we can draw vector orthogonal to this plane. This vector is called _normal_. Graphic card
to draw shadows and colors calculat angle between light source beam and normal of our triangle. That is why order of
points inside triangle is very important. If we have traingle ABC and normal pointed upwards then triangle CBA will have
normal pointe downwards. So final colors of both triangles will be very different!

Now back to polygon. As we said graphic card draws triangles so we must make from our polygon array of triangles. When
we have simple polygon with points ABCDEF than we need to split it into ABC BCD CDE DEF and remember of point order!
We need to have also in mind that we have to handle concave polygon where we can't draw for example CDE triangle but other.
What about holes inside? What about 3D space? This problem is called triangulation and is very important on many fields
from computer graphic to cartographic. Many clever gays try to make easy and fast solution just search on internet
about this problem.

In this plugin we will use library called poly2tri which is implementation of modified constrained 2D Delaunay triangulation.
This algorithm handles well:

* simple polygons
* concave polygons
* polygons with holes

And as always with triangulation better algorithm you choose than longer will it take to return solution.


