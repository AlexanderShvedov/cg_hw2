#include "mainwindow.h"
#include "Window.h"

#include <QSlider>
#include <QFormLayout>
#include <QDockWidget>

namespace
{
constexpr auto g_sampels = 8;
constexpr auto g_gl_major_version = 3;
constexpr auto g_gl_minor_version = 3;
}// namespace


MainWindow::MainWindow()
{
	QSlider* lightXSlider = new QSlider(Qt::Horizontal);
	QLabel* lightXlabel = new QLabel("Light X: ");

	lightXSlider->setMinimum(Window::MIN_COORD);
	lightXSlider->setMaximum(Window::MAX_COORD);
	lightXSlider->setValue(Window::DEFAULT_X);

	QSlider* lightZSlider = new QSlider(Qt::Horizontal);
	QLabel* lightZlabel = new QLabel("Light Z: ");

	lightZSlider->setMinimum(Window::MIN_COORD);
	lightZSlider->setMaximum(Window::MAX_COORD);
	lightZSlider->setValue(Window::DEFAULT_Z);

	QSlider* morphSlider = new QSlider(Qt::Horizontal);
	QLabel* morphLabel = new QLabel("Morph: ");

	morphSlider->setMinimum(Window::MIN_PROGRESS);
	morphSlider->setMaximum(Window::MAX_PROGRESS);
	morphSlider->setValue(00);

	QSlider* sunSlider = new QSlider(Qt::Horizontal);
	QLabel* sunLabel = new QLabel("Sun Light Coef: ");

	sunSlider->setMinimum(Window::MIN_SUN);
	sunSlider->setMaximum(Window::MAX_SUN);
	sunSlider->setValue(Window::DEFAULT_SUN);

	QSlider* ambientSlider = new QSlider(Qt::Horizontal);
	QLabel* ambientLabel = new QLabel("Ambient Light Coef: ");

	ambientSlider->setMinimum(Window::MIN_AMBIENT);
	ambientSlider->setMaximum(Window::MAX_AMBIENT);
	ambientSlider->setValue(Window::DEFAULT_AMBIENT);

	QSlider* spotSlider = new QSlider(Qt::Horizontal);
	QLabel* spotLabel = new QLabel("Spotlight Coef: ");

	spotSlider->setMinimum(Window::MIN_SPOT);
	spotSlider->setMaximum(Window::MAX_SPOT);
	spotSlider->setValue(Window::DEFAULT_SPOT);

	fpsLabel_ = new QLabel();
	fpsLabel_->setText("...");

	QGridLayout* formLayout = new QGridLayout();

	formLayout->addWidget(morphLabel, 0, 0);
	formLayout->addWidget(morphSlider, 0, 1);

	formLayout->addWidget(lightXlabel, 1, 0);
	formLayout->addWidget(lightXSlider, 1, 1);

	formLayout->addWidget(lightZlabel, 2, 0);
	formLayout->addWidget(lightZSlider, 2, 1);

	formLayout->addWidget(spotLabel, 3, 0);
	formLayout->addWidget(spotSlider, 3, 1);

	formLayout->addWidget(ambientLabel, 4, 0);
	formLayout->addWidget(ambientSlider, 4, 1);

	formLayout->addWidget(sunLabel, 5, 0);
	formLayout->addWidget(sunSlider, 5, 1);

	formLayout->addWidget(fpsLabel_, 6, 0);

	QSurfaceFormat format;
	format.setSamples(g_sampels);
	format.setVersion(g_gl_major_version, g_gl_minor_version);
	format.setProfile(QSurfaceFormat::CoreProfile);

	Window* windowWidget = new Window;
	windowWidget->setFormat(format);

	connect(morphSlider, &QSlider::valueChanged, windowWidget, &Window::setMorphingProgress);
	connect(sunSlider, &QSlider::valueChanged, windowWidget, &Window::setSun);
	connect(spotSlider, &QSlider::valueChanged, windowWidget, &Window::setSpot);
	connect(ambientSlider, &QSlider::valueChanged, windowWidget, &Window::setAmbient);
	connect(lightXSlider, &QSlider::valueChanged, windowWidget, &Window::setLightX);
	connect(lightZSlider, &QSlider::valueChanged, windowWidget, &Window::setLightZ);
	connect(windowWidget, &Window::updateFPS, this, &MainWindow::updateFPS);

	setCentralWidget(windowWidget);

	auto dock = new QDockWidget;
	auto settingsWidget = new QWidget;
	settingsWidget->setFocusPolicy(Qt::NoFocus);
	dock->setFocusPolicy(Qt::NoFocus);
	settingsWidget->setLayout(formLayout);
	dock->setWidget(settingsWidget);

	this->addDockWidget(Qt::BottomDockWidgetArea, dock);
}

void MainWindow::updateFPS(uint fps)
{
	fpsLabel_->setText(QString::asprintf("FPS:%u", fps));
}