#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "board.h"


QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

using namespace Chess;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindow *ui;
	Board *m_board;

private:
	void init();

};
#endif // MAINWINDOW_H
