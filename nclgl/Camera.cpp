#include "Camera.h"
#include "Window.h"
#include <algorithm>

void Camera::UpdateCamera(float dt) {

	pitch -= (Window::GetMouse()->GetRelativePosition().y);
	yaw -= (Window::GetMouse()->GetRelativePosition().x);

	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.f);

	if (yaw < 0) {
		yaw += 360.0f;
	}
	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));

	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);

	float speed = 60.0f * dt;
	//float degree = 0.0f;
	if (enableFreeCamera) {
		position += right * speed / 5;
		yaw += speed / 5;
		//if (degree >= 360) {
		//	position += forward * speed;
		//	//degree = 0.0f;
		//}
		//else {
		//	position += right * speed / 5;
		//	yaw += speed / 5;
		//	//degree = speed / 5;
		//}
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
		position += forward * speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
		position -= forward * speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
		position -= right * speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		position += right * speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
		position.y += speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		position.y -= speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_Q)) {
		enableFreeCamera = !enableFreeCamera;
	}
}


Matrix4 Camera::BuildViewMatrix() {
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) * Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) * Matrix4::Translation(-position);
}
