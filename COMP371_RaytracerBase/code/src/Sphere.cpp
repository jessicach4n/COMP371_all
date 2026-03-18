#include "Sphere.h"
#include "Geometry.h"
#include "Ray.h"

Sphere::Sphere(const Eigen::Vector3f& c, float r) : centre(c), radius(r) {}

bool Sphere::intersect(const Ray& ray, HitInfo& hit) const {
	
	// quadratic ray-sphere equation: t^2 + 2(D*(O-C))t + (O-C)*(O-C) - r^2 = 0
	Eigen::Vector3f rayToCenter = ray.getOrigin() - centre;

	float a = ray.getDirection().dot(ray.getDirection());
	float b = 2.0f * ray.getDirection().dot(rayToCenter); // 2(D*(O-C))
	float c = rayToCenter.dot(rayToCenter) - radius * radius; // (O-C)*(O-C) - r^2

	float discriminant = b * b - 4 * a * c;

	if (discriminant < 0.0f) return false; // no intersection

	float sqrtDisc = std::sqrt(discriminant);

	float t1 = (-b - sqrtDisc) / (2 * a); // smaller root
	float t2 = (-b + sqrtDisc) / (2 * a); // larger root

	float t_hit;
	if (t1 > 0)
		t_hit = t1; // pick closest positive
	else if (t2 > 0)
		t_hit = t2; // ray starts inside sphere, pick exiting intersection
	else
		return false; // both intersections are behind the ray

	// update hit if this intersection is closer than any previous hit
	if (t_hit < hit.t) {
		hit.t = t_hit;
		hit.position = ray.getPointAt(t_hit);
		hit.normal = (hit.position - centre).normalized();
		hit.geometry = static_cast<const Geometry*>(this);
		return true;
	}

	return false;

}