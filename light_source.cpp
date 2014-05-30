/***********************************************************
    Starter code for Assignment 3

    This code was originally written by Jack Wang for
   CSC418, SPRING 2005

implements light_source.h

***********************************************************/


#include <cmath>
#include "light_source.h"
#include <iostream>

//extern bool inShadow;
extern bool specularOff;

Colour PointLight::get_ambient(Material* mat) {
   return mat->ambient * _col_ambient ;
}


Colour PointLight::get_diffuse(Material* mat, Vector3D& normal, Vector3D& dirLight) {
    return  fmax(0.0, normal.dot(dirLight)) * mat->diffuse * _col_diffuse;
}


Colour PointLight::get_specular(Material* mat, Vector3D& normal, Vector3D& dirLight, Vector3D& toViewer) {
   Vector3D r = 2 * (normal.dot(dirLight)) * normal - dirLight;
   r.normalize();
     return fmax(0, pow(r.dot(toViewer),mat->specular_exp))*mat->specular* _col_specular;
   //return  std::pow(fmax(0.0, r.dot(toViewer)), mat->specular_exp)*(mat->specular * _col_specular);
   //changed the return expression a little bit - vega

}

void PointLight::shade( Ray3D& ray, bool inShadow) {
// TODO: implement this function to fill in values for ray.col
// using phong shading.  Make sure your vectors are normalized, and
// clamp colour values to 1.0.
   //
   // in util.cpp, use:
   // normalizing:         double Vector3D::normalize()
   // clamping colours:    void Colour::clamp()
//
// It is assumed at this point that the intersection information in ray
// is available.  So be sure that traverseScene() is called on the ray
// before this function.  
   Material* mat = ray.intersection.mat;
   
   
   Vector3D dirLight = get_position() - ray.intersection.point;
   dirLight.normalize();
   
   Vector3D normal = ray.intersection.normal;
   normal.normalize();
   
   Vector3D toViewer = -ray.dir;
   toViewer.normalize();
   
   Colour ambient  = get_ambient(mat);
   Colour diffuse  = get_diffuse(mat, normal, dirLight);
   Colour specular = get_specular(mat, normal, dirLight, toViewer);
   
  if(inShadow)
  {

   // Colour c(0.0,0.0,0.0);
    ray.col = ambient;

  }
  else
  {
    
   if (specularOff)
      ray.col = ambient + diffuse;
   else        
	 ray.col = ambient + diffuse + specular;    

  }
   ray.col.clamp();
}

/*Ray3D PointLight::shadow( Ray3D& ray){

    Ray3D newRay;
    newRay.dir = get_position() - ray.intersection.point;
    newRay.dir.normalize();
    newRay.origin = ray.intersection.point + 0.01*newRay.dir;
    
    Point3D lightOri = ray.origin;
    Point3D intersectionPoint = ray.intersection.point;
    
    Vector3D d1 = newRay.origin - intersectionPoint ;    
    Vector3D d2 = newRay.origin - lightOri; 
    
    if(!ray.intersection.none)
    {
      //to the intersection point
      double v1 = (d1[0])*(d1[0]) + (d1[1])*(d1[1]) + (d1[2])*(d1[2]);
      //distance to the light source
      double v2 = (d2[0])*(d2[0]) + (d2[1])*(d2[1]) + (d2[2])*(d2[2]);

      if(v1 <= v2 )
	inShadow = true;
      else
	inShadow = false;      
    }
    else 
      inShadow = false;

    return newRay;
}

*/