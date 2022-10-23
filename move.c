#include "minirt.h"

void transpose_obj_step(t_minirt *data, int type);

int	ft_close(t_minirt *data)
{
	t_light *light;
	t_objs *obj;

	mlx_clear_window(data->mlx.mlx, data->mlx.mlx_win);
	mlx_destroy_window(data->mlx.mlx, data->mlx.mlx_win);
	while (data->scene.light)
	{
		light = data->scene.light;
		data->scene.light = data->scene.light->next;
		free(light);
	}
	while (data->scene.objs)
	{
		obj = data->scene.objs;
		data->scene.objs = data->scene.objs->next;
		free(obj);
	}
	// system("leaks miniRT");
	exit(0);
}

t_vec rotate(t_vec axis, t_minirt* vars, int dir)
{
	double c = (1 - cos(dir * 0.1));
	double s = sin(dir * 0.1);
	double x = axis.x;
	double y = axis.y;
	double z = axis.z;

	double i = vars->scene.camera.forward.x;
	double j = vars->scene.camera.forward.y;
	double k = vars->scene.camera.forward.z;

	t_vec new_dir;

	new_dir.x = -i*c*y*y-k*s*y+c*j*x*y-i*c*z*z+j*s*z+c*k*x*z+i;
	new_dir.y = j-c*j*x*x+k*s*x+i*c*x*y-c*j*z*z-i*s*z+c*k*y*z;
	new_dir.z = k-c*k*x*x-j*s*x-c*k*y*y+i*s*y+i*c*x*z+c*j*y*z;

	return (new_dir);
}

void camera_move(t_minirt* vars)
{
	t_vec dir;
	t_vec delta;
	double d;

	if (vars->is_move == 13 || vars->is_move == 1)
	{
		dir = vars->scene.camera.forward;
		if (vars->is_move == 13)
			d = 1;
		else
			d = -1;
	}
	else if (vars->is_move == 0 || vars->is_move == 2)
	{
		dir = vars->scene.camera.right;
		if (vars->is_move == 2)
			d = 1;
		else
			d = -1;
	}
	else
		return ;
	delta = vec_scalar_mul(vec_scalar_mul(dir, d), vars->scene.camera.distance / 10);
	t_vec new_org = vec_sum(vars->scene.camera.origin, delta);
	vars->scene.camera.origin = new_org;
}

void camera_rotate(t_minirt* vars)
{
	t_vec axis;
	t_vec delta;
	double d;

	if (vars->is_move == 126 || vars->is_move == 125)
	{
		axis = vars->scene.camera.right;
		if (vars->is_move == 126)
			d = -1;
		else
			d = 1;
	}
	else if (vars->is_move == 123 || vars->is_move == 124)
	{
		axis = vars->scene.camera.up;
		if (vars->is_move == 124)
			d = 1;
		else
			d = -1;
	}
	else
		return ;
	t_vec new_dir = rotate(axis, vars, d);
	vars->scene.camera.forward = new_dir;
	vars->scene.camera.dir = new_dir;
}

void camera_zoom(t_minirt* vars)
{
	double new_fov;

	if (vars->is_move == 4 || vars->is_move == 5)
	{
		if ((vars->is_move == 4 && vars->scene.camera.fov <= 10)
			|| (vars->is_move == 5 && vars->scene.camera.fov >= 170))
		{
			printf("cannot zoom more\n");
			vars->is_move = -1;
			return ;
		}
		if (vars->is_move == 4)
			new_fov = vars->scene.camera.fov - 10;
		else
			new_fov = vars->scene.camera.fov + 10;
		vars->scene.camera.fov = new_fov;
		vars->is_move = -1;
	}
}

int key_hook_move(t_minirt* vars)
{
	if (vars->scene.changed == 1)
	{
		if (vars->is_trace == 1 || vars->is_trace == 0)
			path_render(*vars);
		else if (vars->is_trace == 2)
			rt_render(vars);
		vars->scene.changed = 0;
	}
	if (vars->is_trace == 0 && vars->is_move != -1)
	{
		if (vars->mode == 0)
		{
			camera_move(vars);
			camera_rotate(vars);
			camera_zoom(vars);
			set_camera(&vars->scene.camera);
		}
		else if (vars->mode != 0)
		{
			transpose_obj_step(vars, vars->mode);
		}
		path_render(*vars);
	}
	return (1);
}

void transpose_obj_step(t_minirt *data, int type)
{
	t_vec dir;
	t_vec delta;
	t_objs *tmp;
	double d;

	if (data->is_move == 13 || data->is_move == 1)
	{
		dir = data->scene.camera.up;
		if (data->is_move == 13)
			d = 1;
		else
			d = -1;
	}
	else if (data->is_move == 0 || data->is_move == 2)
	{
		dir = data->scene.camera.right;
		if (data->is_move == 2)
			d = 1;
		else
			d = -1;
	}
	else
		return ;
	delta = vec_scalar_mul(vec_scalar_mul(dir, d), 1);
	tmp = data->scene.objs;
	while (tmp)
	{
		if (tmp->type == type)
			tmp->center = vec_sum(tmp->center, delta);
		tmp = tmp->next;
	}
}

int transpose_light(t_minirt *data, t_keycode keycode, int *status)
{
	t_light *light = data->scene.light;

	*status = -1;
	if (keycode == W)
		light->src.y += STEP;
	else if (keycode == A)
		light->src.x += STEP;
	else if (keycode == D)
		light->src.z += STEP;
	rt_render(data);
	return (0);
}

void rotate_obj_step(t_minirt *data, int pos1, int pos2, int type)
{
	t_objs *tmp;
	double 	pos[3];
	double	r_pos[3];

	tmp = data->scene.objs;
	while (tmp)
	{
		if (tmp->type == type)
		{
			r_pos[0] = tmp->dir.x;
			r_pos[1] = tmp->dir.y;
			r_pos[2] = tmp->dir.z;
			pos[0] = tmp->dir.x;
			pos[1] = tmp->dir.y;
			pos[2] = tmp->dir.z;
			r_pos[pos1] = pos[pos1] * cos(ROTATE) - pos[pos2] * sin(ROTATE);
			r_pos[pos2] = pos[pos1] * sin(ROTATE) + pos[pos2] * cos(ROTATE);
			set_vec(&tmp->dir, r_pos[0], r_pos[1], r_pos[2]);
		}
		tmp = tmp->next;
	}
	rt_render(data);
}

int rotate_obj(t_minirt *data, t_keycode keycode, int type, int *status)
{
	t_objs *tmp;
	t_vec ori;

	*status = -1;
	tmp = data->scene.objs;
	if (keycode == W) // y축 중심 회전
		rotate_obj_step(data, 2, 0, type);
	else if (keycode == A) // x축 중심 회전
		rotate_obj_step(data, 1, 2, type);
	if (keycode == D) // z축 중심 회전
		rotate_obj_step(data, 0, 1, type);
	return (0);
}

int	keypress(int keycode, t_minirt* vars)
{
	if (keycode == W || keycode == A || keycode == S || keycode == D)
		key_press_move(vars, keycode);
	else if (keycode == UP || keycode == LEFT || keycode == RIGHT || keycode == DOWN)
		key_press_rotate(vars, keycode);
	else if (keycode == 15 || keycode == 35 || keycode == 18 || keycode == 19)
		key_press_mode_change(vars, keycode);
	return (0);
}

int	keyrelease(int keycode, t_minirt* vars)
{
	// printf("key release=%d\n", keycode);
	if (keycode == 13)
		vars->is_move = -1;
	else if (keycode == 0)
		vars->is_move = -1;
	else if (keycode == 1)
		vars->is_move = -1;
	else if (keycode == 2)
		vars->is_move = -1;
	else if (keycode == 126)
		vars->is_move = -1;
	else if (keycode == 123)
		vars->is_move = -1;
	else if (keycode == 125)
		vars->is_move = -1;
	else if (keycode == 124)
		vars->is_move = -1;
	return (0);
}

int scroll(int mousecode, int x, int y, t_minirt* vars)
{
	if (vars->is_trace == 1)
		printf("cannot zoom here\n");
	else if (mousecode == 4 || mousecode == 5)
		vars->is_move = mousecode;
	else if (mousecode == 1)
		printf("%d , %d\n", vars->x, vars->y);
	return (0);
}

void key_press_move(t_minirt* vars, int keycode)
{
	if (vars->is_trace == 1)
		printf("cannot move here\n");
	else
		vars->is_move = keycode;
}

void key_press_rotate(t_minirt* vars, int keycode)
{
	if (vars->is_trace == 1)
		printf("cannot rotate here\n");
	else
		vars->is_move = keycode;
}

void key_press_mode_change(t_minirt* vars, int keycode)
{
	if (vars->is_trace == 0)
	{
		if(keycode == 18 || keycode == 19)
		{
			if (vars->mode == 0)
			{
				if (keycode == 18)
					vars->mode = SP;
				else if (keycode == 19)
					vars->mode = CY;
			}
			else if (vars->mode != 0)
				vars->mode = 0;
			return ;
		}

		if (keycode == 15)
			vars->is_trace = 1;
		else if (keycode == 35)
			vars->is_trace = 2;
		vars->scene.anti = 1;
		vars->scene.changed = 1;
	}
	else
	{
		vars->is_trace = 0;
		vars->scene.anti = 1;
		vars->scene.changed = 1;
	}
}