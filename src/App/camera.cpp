#include "camera.h"

#include <App/thirdparty/glm/glm/glm.hpp>
#include <App/thirdparty/glm/glm/ext.hpp>
#include <App/thirdparty/glm/glm/gtx/rotate_vector.hpp>

Camera::Camera(size_t width, size_t height, QVector3D position)
{
	Camera::width = width;
	Camera::height = height;
	Camera::aspect = static_cast<float>(width) / static_cast<float>(height);
	Camera::position = position;
}

glm::vec3 toGLMVec3(QVector3D in) {
	return {in.x(), in.y(), in.z()};
}

QVector3D toQVec3(glm::vec3 in) {
	return {in.x, in.y, in.z};
}

std::tuple<QMatrix4x4, QMatrix4x4, QMatrix4x4, QVector3D> Camera::update(float fovd, float near, float far, size_t totalFrameCount_)
{
	// Calculate MVP matrix
	model.setToIdentity();

	glm::vec3 glmOrientation = toGLMVec3(orientation);
	glm::vec3 glmUp = toGLMVec3(up);
	glm::vec3 glmNewOrientation = glm::rotate(glmOrientation, glm::radians(-rotationX), glm::normalize(glm::cross(glmOrientation, glmUp)));
	glmNewOrientation = glm::rotate(glmNewOrientation, glm::radians(-rotationY), glmUp);
	rotationX = 0.0f;
	rotationY = 0.0f;
	orientation = toQVec3(glmNewOrientation);

	view.setToIdentity();
	view.lookAt(position, position + orientation, up);

	QVector4D newRight4D = QVector4D(right, 1.0) * view;
	QVector3D newRight = newRight4D.toVector3D().normalized();

	// view.translate(position);

	position += movement.z() * orientation;
	position += movement.x() * newRight;
	position += {0.0, movement.y(), 0.0};
	movement = {0.0, 0.0, 0.0};

	projection.setToIdentity();
	projection.perspective(fovd, aspect, near, far);

	return {model, view, projection, orientation};
}

template <typename T> float sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

void Camera::mousePressEvent(QMouseEvent *event)
{
	cord = {static_cast<float>(event->x()), static_cast<float>(event->y())};
}

void Camera::input(QMouseEvent *event)
{
	if(event->buttons() == Qt::LeftButton)
	{
		QVector2D change = {-sgn(event->x() - cord.x()), sgn(event->y() - cord.y())};
		cord = {static_cast<float>(event->x()), static_cast<float>(event->y())};
		rotationX += change.x() * 0.01;
		orientation += QVector3D{0.0f, change.y(), -change.x()} * 0.01;
	}
}

void Camera::resize(size_t width, size_t height)
{
	this->aspect = static_cast<float>(width) / static_cast<float>(height);
}
void Camera::wheelEvent(QWheelEvent * event)
{
	if (event->angleDelta().y() > 0) {
		movement += {0.0, 0.0, 3 * speed};
	} else if (event->angleDelta().y() < 0) {
		movement += {0.0, 0.0, 3 * -speed};
	}
}
