#ifndef CAMERA_H
#define CAMERA_H
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>

class Camera
{
public:
	QVector2D cord;
	QVector3D position{0.0f, 0.0f, 1.5f};
	QVector3D orientation{0.0f, 0.0f, 1.0f};
	QVector3D up{0.0f, 2.0f, 0.0f};
	QVector3D right{1.0f, 0.0f, 0.0f};

	size_t width, height;

	QVector3D localOrientation{1.0f, 0.0f, -1.0f};
	float rotationX = 0, rotationY = 100;
	QVector3D movement{0.0f, 0.0f, 0.0f};

	bool firstClick = true;
	QPointF prevPos;

	float aspect = 1.0f;

	float speed = 0.1f;
	float sensitivity = 0.1f;

	QMatrix4x4 model;
	QMatrix4x4 view;
	QMatrix4x4 projection;

	Camera() = default;
	Camera(size_t width, size_t height, QVector3D position);

	std::tuple<QMatrix4x4, QMatrix4x4, QMatrix4x4, QVector3D> update(float fovd, float near, float far, size_t totalFrameCount_);
	void input(QMouseEvent* event);
	void wheelEvent(QWheelEvent *event);
	void resize(size_t width, size_t height);
	void mousePressEvent(QMouseEvent * event);
};

#endif // CAMERA_H