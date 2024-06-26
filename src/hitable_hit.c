/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hitable_hit.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/07 21:50:15 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/04/24 01:58:07 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"
#include "printf/ft_printf_bonus.h"
#include "vec3.h"
#include <stdio.h>

t_hitpoint	hit_plane(t_hitable plane, t_ray ray)
{
	t_hitpoint	hp;

	hp.hit = 0;
	if (vec3_dot(plane.normal, ray.dir) == 0
		|| vec3_dot(vec3_unit(vec3_sub(plane.pos, *ray.origin)), plane.normal) == 0)
		return (hp);
	if(vec3_dot(ray.dir, plane.normal) > 0)
		plane.normal = vec3_scale(plane.normal, -1);
	hp.distance = vec3_dot(vec3_sub(plane.pos, *ray.origin), plane.normal)
		/ vec3_dot(ray.dir, plane.normal);
	if (hp.distance <= 0)
		return (hp);
	hp.hit = 1;
	hp.surface_normal_of_hittable = plane.normal;
	hp.color = plane.color;
	hp.pos = vec3_add(*ray.origin, vec3_scale(ray.dir, hp.distance));
	return (hp);
}

t_hitpoint	hit_circle(t_hitable plane, t_ray ray, float radius)
{
	t_hitpoint hp;

	hp = hit_plane(plane, ray);
	hp.color = scale_color(hp.color, 0.7);
	if (hp.hit && vec3_distance(hp.pos, plane.pos) <= radius)
		return (hp);
	hp.hit = 0;
	return (hp);
}

t_hitpoint	hit_cylinder_side(t_hitable cylinder, t_ray ray)
{
	float d;
	t_hitpoint hp;
	t_vector3 b = vec3_sub(cylinder.pos, *ray.origin);
	t_vector3 cross = vec3_cross(ray.dir, cylinder.normal);
	t_vector3 cross1 = vec3_cross(cylinder.normal, ray.dir);
	t_vector3 crossbcnormal = vec3_cross(b, cylinder.normal);
	float disc = vec3_dot(cross, cross) * pow(cylinder.diameter / 2, 2) - pow(vec3_dot(b, cross), 2); 

	hp.hit = 0;
	if (disc < 0)
		return (hp);
	hp.distance = (vec3_dot(cross, crossbcnormal) - sqrt(disc)) / vec3_dot(cross1, cross1);
	if (hp.distance > 0)
		hp.hit = calc_vertex_normal(cylinder, ray, &hp, atan((cylinder.diameter / 2) / cylinder.height));
	d = (vec3_dot(cross, crossbcnormal) + sqrt(disc)) / vec3_dot(cross1, cross1);
	if (hp.distance > d && d > 0)
	{
		hp.distance = d;
		hp.hit = calc_vertex_normal(cylinder, ray, &hp, atan((cylinder.diameter / 2) / cylinder.height));
	}
	hp.color = cylinder.color;
	return (hp);
}

t_hitpoint	hit_cylinder(t_hitable cylinder, t_ray ray)
{
	t_hitpoint	hit_side;
	t_hitpoint	hit_cap1;
	t_hitpoint	hit_cap2;
	t_hitpoint	hp;

	hit_side = hit_cylinder_side(cylinder, ray);
	hit_cap2 = hit_circle((t_hitable){
			'p', cylinder.pos, cylinder.normal, 1, 1, cylinder.color, NULL	}, ray, cylinder.diameter / 2);	
	hit_cap1 = hit_circle((t_hitable){
			'p', vec3_add(cylinder.pos, vec3_scale(cylinder.normal, cylinder.height)),
			cylinder.normal, 1, 1, cylinder.color, NULL	}, ray, cylinder.diameter / 2);
	hp = hit_side;
	if (hit_cap1.hit && (!hp.hit || hp.distance > hit_cap1.distance))
		hp = hit_cap1;
	if (hit_cap2.hit && (!hp.hit || hp.distance > hit_cap2.distance))
		hp = hit_cap2;	
	return (hp);
}

t_hitpoint	hit_sphere(t_hitable sphere, t_ray ray)
{
	t_discrimininant	dis;
	t_hitpoint			hp;
	
	hp.hit = 0;
	dis.oc = vec3_sub(*ray.origin, sphere.pos);
	dis.a = vec3_dot(ray.dir, ray.dir);
	dis.b = 2.0 * vec3_dot(dis.oc, ray.dir);
	dis.c = vec3_dot(dis.oc, dis.oc) - sphere.diameter / 2 * sphere.diameter / 2;
	dis.discriminant = dis.b * dis.b - 4 * dis.a * dis.c;
	if (dis.discriminant < 0)
		return (hp);
	hp.distance = (-dis.b - sqrt(dis.discriminant)) / (2 * dis.a);
	if (hp.distance < 0)
		return (hp);
	hp.hit = 1;
	hp.pos = vec3_add(*ray.origin, vec3_scale(ray.dir, hp.distance));
	hp.color = sphere.color;
	hp.surface_normal_of_hittable = vec3_unit(vec3_sub(hp.pos, sphere.pos));
	return (hp);
}

t_hitpoint	hit_hitable(t_hitable *list, t_ray ray)
{
	t_hitable	*tmp;
	t_hitpoint	hp;
	t_hitpoint	tmp_hp;

	hp.hit = 0;
	tmp = list;
	while (tmp)
	{
		tmp_hp.hit = 0;
		if (tmp->type == 's')
			tmp_hp = hit_sphere(*tmp, ray);
		if (tmp->type == 'c')
			tmp_hp = hit_cylinder(*tmp, ray);
		if (tmp->type == 'p')
			tmp_hp = hit_plane(*tmp, ray);
		if (tmp->type == 'o')
			tmp_hp = hit_cone(*tmp, ray);
		if (tmp_hp.hit && tmp_hp.distance > 0 && (!hp.hit || tmp_hp.distance < hp.distance))
			hp = tmp_hp;
		tmp = tmp->next;
	}
	return (hp);
}
