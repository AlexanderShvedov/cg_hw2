#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow();
public slots:
	void updateFPS(uint);
private:
	QLabel* fpsLabel_;
};

#endif // MAINWINDOW_H