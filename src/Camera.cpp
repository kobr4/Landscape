//camera.cpp
#include "camera.h"
#include <stdio.h>
using namespace std;

const int Camera::deadzone = 400;

Camera::Camera() {
	camera_mode = FREE;
	camera_up = glm::vec3(0, 1, 0);
	field_of_view = 90;
	camera_position_delta = glm::vec3(0, 0, 0);
	camera_scale = .5f;
	max_pitch_rate = 5;
	max_heading_rate = 5;
	move_camera = false;
	camera_look_at = glm::vec3();
	camera_position = glm::vec3();
	camera_pitch = 0.f;
	xjaxis = 0;
	yjaxis = 0;
	zjaxis = 0;
	wjaxis = 0;
	near_clip = 1.0f;
	far_clip = 100.0f;
	aspect = 16.0/9.0;

}
Camera::~Camera() {
}

void Camera::Reset() {
	camera_up = glm::vec3(0, 1, 0);
}


glm::vec3 Camera::getMotionHeading() {
	return glm::vec3(glm::normalize(camera_position_delta));
}

void Camera::resetMotionHeading() {
	camera_position_delta = glm::vec3(0.0f,0.0f,0.0f);
}

void Camera::Update() {
	this->UpdateInput();

	camera_direction = glm::normalize(camera_look_at - camera_position);
	//printf("delta %f %f %f\n",camera_direction[0],camera_direction[1], camera_direction[2]);
	//need to set the matrix state. this is only important because lighting doesn't work if this isn't done
	//glViewport(viewport_x, viewport_y, window_width, window_height);

	if (camera_mode == ORTHO) {
		//our projection matrix will be an orthogonal one in this case
		//if the values are not floating point, this command does not work properly
		//need to multiply by aspect!!! (otherise will not scale properly)
		projection = glm::ortho(-1.5f * float(aspect), 1.5f * float(aspect), -1.5f, 1.5f, -10.0f, 10.f);
	} else if (camera_mode == FREE) {
		
		projection = glm::perspective(field_of_view, aspect, near_clip, far_clip);
		//detmine axis for pitch rotation
		glm::vec3 axis = glm::cross(camera_direction, camera_up);
		//compute quaternion for pitch based on the camera pitch angle
		glm::quat pitch_quat = glm::angleAxis(camera_pitch, axis);
		//determine heading quaternion from the camera up vector and the heading angle
		glm::quat heading_quat = glm::angleAxis(camera_heading, camera_up);
		//add the two quaternions
		glm::quat temp = glm::cross(pitch_quat, heading_quat);
		temp = glm::normalize(temp);
		//update the direction from the quaternion
		camera_direction = glm::rotate(temp, camera_direction);
		//add the camera delta
		camera_position += camera_position_delta;
		//set the look at to be infront of the camera
		camera_look_at = camera_position + camera_direction * 1.0f;
		//damping for smooth camera
		camera_heading *= .5;
		camera_pitch *= .5;
		camera_position_delta = camera_position_delta * .8f;
		
	}
	//compute the MVP
	view = glm::lookAt(camera_position, camera_look_at, camera_up);
	model = glm::mat4(1.0f);
	//MVP = projection * view * model;
}

glm::mat4 Camera::TranslateToEye(float ipdFactor) {
	glm::vec3 camera_position_offset = camera_position + glm::cross(camera_direction, camera_up) * ipdFactor;
	glm::vec3 camera_look_at_offset = camera_position_offset + camera_direction * 1.0f;
	glm::mat4 view_offset = glm::lookAt(camera_position_offset, camera_look_at_offset, camera_up);
	return view_offset;
}

//Setting Functions
void Camera::SetMode(CameraType cam_mode) {
	camera_mode = cam_mode;
	camera_up = glm::vec3(0, 1, 0);
}

void Camera::SetPosition(glm::vec3 pos) {
	camera_position = pos;
}

void Camera::SetLookAt(glm::vec3 pos) {
	camera_look_at = pos;
}
void Camera::SetFOV(double fov) {
	field_of_view = fov;
}
void Camera::SetViewport(int loc_x, int loc_y, int width, int height) {
	viewport_x = loc_x;
	viewport_y = loc_y;
	window_width = width;
	window_height = height;
	//need to use doubles division here, it will not work otherwise and it is possible to get a zero aspect ratio with integer rounding
	aspect = double(width) / double(height);
	;
}
void Camera::SetClipping(double near_clip_distance, double far_clip_distance) {
	near_clip = near_clip_distance;
	far_clip = far_clip_distance;
}

void Camera::Move(CameraDirection dir, float factor) {
	if (camera_mode == FREE) {
		switch (dir) {
			case UP:
				camera_position_delta += camera_up * camera_scale * factor;
				break;
			case DOWN:
				camera_position_delta -= camera_up * camera_scale * factor;
				break;
			case LEFT:
				camera_position_delta -= glm::cross(camera_direction, camera_up) * camera_scale * factor;
				break;
			case RIGHT:
				camera_position_delta += glm::cross(camera_direction, camera_up) * camera_scale * factor;
				break;
			case FORWARD:
				camera_position_delta += camera_direction * camera_scale * factor;
				break;
			case BACK:
				camera_position_delta -= camera_direction * camera_scale * factor;
				break;
		}
	}

}
void Camera::ChangePitch(float degrees) {
	//Check bounds with the max pitch rate so that we aren't moving too fast
	if (degrees < -max_pitch_rate) {
		degrees = -max_pitch_rate;
	} else if (degrees > max_pitch_rate) {
		degrees = max_pitch_rate;
	}
	camera_pitch += degrees;

	//Check bounds for the camera pitch
	if (camera_pitch > 360.0f) {
		camera_pitch -= 360.0f;
	} else if (camera_pitch < -360.0f) {
		camera_pitch += 360.0f;
	}
}
void Camera::ChangeHeading(float degrees) {
	//Check bounds with the max heading rate so that we aren't moving too fast
	if (degrees < -max_heading_rate) {
		degrees = -max_heading_rate;
	} else if (degrees > max_heading_rate) {
		degrees = max_heading_rate;
	}
	//This controls how the heading is changed if the camera is pointed straight up or down
	//The heading delta direction changes
	if (camera_pitch > 90 && camera_pitch < 270 || (camera_pitch < -90 && camera_pitch > -270)) {
		camera_heading -= degrees;
	} else {
		camera_heading += degrees;
	}
	//Check bounds for the camera heading
	if (camera_heading > 360.0f) {
		camera_heading -= 360.0f;
	} else if (camera_heading < -360.0f) {
		camera_heading += 360.0f;
	}
}
void Camera::Move2D(int x, int y) {
	//compute the mouse delta from the previous mouse position
	glm::vec3 mouse_delta = mouse_position - glm::vec3(x, y, 0);
	//if the camera is moving, meaning that the mouse was clicked and dragged, change the pitch and heading
	
	//if (move_camera) {
		ChangeHeading(.16f * mouse_delta.x);
		ChangePitch(.16f * mouse_delta.y);
	//}
	mouse_position = glm::vec3(x, y, 0);
}

void Camera::Move2DJoy(int x, int y) {
	glm::vec3 mouse_delta = -glm::vec3(x, y, 0);

	ChangeHeading(.16f * mouse_delta.x);
	ChangePitch(.16f * mouse_delta.y);
	
	mouse_position = glm::vec3(x, y, 0);
}

void Camera::MoveOvr(float pitch, float yaw, float roll) {
	const float ovrScale = 1.f;
	glm::vec3 ovr_delta = ovr_position - glm::vec3(pitch,yaw,roll);


	ChangeHeading(ovrScale * ovr_delta.y);
	ChangePitch(ovrScale * ovr_delta.x);

	ovr_position = glm::vec3(pitch,yaw,roll);
}


/*
void Camera::SetPos(int button, int state, int x, int y) {
	if (button == 3 && state == GLUT_DOWN) {
		camera_position_delta += camera_up * .05f;
	} else if (button == 4 && state == GLUT_DOWN) {
		camera_position_delta -= camera_up * .05f;
	} else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		move_camera = true;
	} else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		move_camera = false;
	}
	mouse_position = glm::vec3(x, y, 0);
}
*/

CameraType Camera::GetMode() {
	return camera_mode;
}

void Camera::GetViewport(int &loc_x, int &loc_y, int &width, int &height) {
	loc_x = viewport_x;
	loc_y = viewport_y;
	width = window_width;
	height = window_height;
}

void Camera::GetMatricies(glm::mat4 &P, glm::mat4 &V, glm::mat4 &M) {
	P = projection;
	V = view;
	M = model;
}

glm::vec3 Camera::getPosition() {
	return camera_position;
}

void Camera::UpdateInput() {

	if (xjaxis < -deadzone) {
		this->Move(CameraDirection::LEFT, -(float)(xjaxis)/32768.f);
	} else if (xjaxis > deadzone){
		this->Move(CameraDirection::RIGHT, (float)(xjaxis)/32768.f);
	}

	if (yjaxis < -deadzone) {
		this->Move(CameraDirection::FORWARD, -(float)(yjaxis)/32768.f);
	} else if (yjaxis > deadzone){
		this->Move(CameraDirection::BACK, (float)(yjaxis)/32768.f);
	}

	if (zjaxis != 0 && wjaxis != 0) {
		this->Move2DJoy(zjaxis/500,wjaxis/500);
	}
}

void Camera::handleEvent(SDL_Event event) {
	float factor = 10.f;
	switch( event.type )
	{
		case SDL_KEYDOWN:
			switch( event.key.keysym.sym )
			{
				case SDLK_SPACE :
					break;
				case SDLK_UP:
					this->Move(CameraDirection::FORWARD,factor);
					break;
				case SDLK_DOWN:
					this->Move(CameraDirection::BACK,factor);
					break;
				case SDLK_LEFT:
					this->Move(CameraDirection::LEFT);
					break;
				case SDLK_RIGHT:
					this->Move(CameraDirection::RIGHT);
					break;
				default:
					break;
			}
		break;
		case SDL_MOUSEMOTION:
			this->Move2D(event.motion.x,event.motion.y);
			break;
		case SDL_JOYAXISMOTION:
			if (event.jaxis.axis == 0) {
				xjaxis = event.jaxis.value / 10;
			}

			if (event.jaxis.axis == 1) {
				yjaxis = event.jaxis.value / 10;
			}

			if (event.jaxis.axis == 2) {
				zjaxis = event.jaxis.value / 10;
			}

			if (event.jaxis.axis == 3) {
				wjaxis = event.jaxis.value / 10;
			}
			break;
	}               
}