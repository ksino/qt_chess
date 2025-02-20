#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QGroupBox>
#include <QTextEdit>
#include <QLabel>

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
	QWidget *centralWidget;
	QHBoxLayout *mainLayout;
	QWidget *chessWidget;
	QVBoxLayout *rightLayout;
	QGroupBox *whoFirstGroup;
	QVBoxLayout *whoFirstLayout;
	QRadioButton *humanFirst;
	QRadioButton *computerFirst;
	QRadioButton *noComputer;
	QPushButton *restartButton;
	QPushButton *undoButton;
	QGroupBox *computerLevelGroup;
	QVBoxLayout *computerLevelLayout;
	QRadioButton *amateurLevel;
	QRadioButton *beginnerLevel;
	QRadioButton *professionalLevel;
	QGroupBox *soundGroup;
	QVBoxLayout *soundLayout;
	QRadioButton *soundOn;
	QRadioButton *soundOff;
	QGroupBox *animationGroup;
	QVBoxLayout *animationLayout;
	QRadioButton *animationOn;
	QRadioButton *animationOff;
	QTextEdit *moveHistory;
	Board *m_board;

private:
	void init();
	void setupUi();

};
#endif // MAINWINDOW_H
