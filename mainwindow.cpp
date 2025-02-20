#include "mainwindow.h"
#include "const_data.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setupUi();
//	this->init();
}

MainWindow::~MainWindow()
{
	delete m_board;
}

void MainWindow::init()
{
//	this->setWindowTitle(cszAbout);
//	//新建棋盘实例
//	m_board = new Board(ui->centralwidget);
//	//设定棋盘大小和坐标
//	m_board->setGeometry(QRect(57, 57, BOARD_WIDTH, BOARD_HEIGHT));
}

void MainWindow::setupUi()
{
	centralWidget = new QWidget(this);
	setCentralWidget(centralWidget);
	mainLayout = new QHBoxLayout(centralWidget);

	// 左边显示中国象棋的背景
	chessWidget = new QWidget(this);
	this->setWindowTitle(cszAbout);
	//新建棋盘实例
	m_board = new Board(chessWidget);
	//设定棋盘大小和坐标
	m_board->setGeometry(QRect(FILE_LEFT, RANK_TOP, BOARD_WIDTH, BOARD_HEIGHT + RANK_BOTTOM));
	mainLayout->addWidget(chessWidget, 8);

	// 右边布局
	rightLayout = new QVBoxLayout();

	// 1. 三个选项“谁先走，电脑先走，不用电脑”
	whoFirstGroup = new QGroupBox("谁先走", this);
	whoFirstLayout = new QVBoxLayout();
	humanFirst = new QRadioButton("玩家先走", this);
	computerFirst = new QRadioButton("电脑先走", this);
	noComputer = new QRadioButton("不用电脑", this);
	whoFirstLayout->addWidget(humanFirst);
	whoFirstLayout->addWidget(computerFirst);
	whoFirstLayout->addWidget(noComputer);
	whoFirstGroup->setLayout(whoFirstLayout);
	rightLayout->addWidget(whoFirstGroup);

	// 2. 重新开始按钮
	restartButton = new QPushButton("重新开始", this);
	rightLayout->addWidget(restartButton);

	// 3. 悔棋按钮
	undoButton = new QPushButton("悔棋", this);
	rightLayout->addWidget(undoButton);

	// 4. 电脑水平选择，业余、入门、专业
	computerLevelGroup = new QGroupBox("电脑水平", this);
	computerLevelLayout = new QVBoxLayout();
	amateurLevel = new QRadioButton("业余", this);
	beginnerLevel = new QRadioButton("入门", this);
	professionalLevel = new QRadioButton("专业", this);
	computerLevelLayout->addWidget(amateurLevel);
	computerLevelLayout->addWidget(beginnerLevel);
	computerLevelLayout->addWidget(professionalLevel);
	computerLevelGroup->setLayout(computerLevelLayout);
	rightLayout->addWidget(computerLevelGroup);

	// 5. 音效单选框
	soundGroup = new QGroupBox("音效", this);
	soundLayout = new QVBoxLayout();
	soundOn = new QRadioButton("开启", this);
	soundOff = new QRadioButton("关闭", this);
	soundLayout->addWidget(soundOn);
	soundLayout->addWidget(soundOff);
	soundGroup->setLayout(soundLayout);
	rightLayout->addWidget(soundGroup);

	// 6. 动画单选框
	animationGroup = new QGroupBox("动画", this);
	animationLayout = new QVBoxLayout();
	animationOn = new QRadioButton("开启", this);
	animationOff = new QRadioButton("关闭", this);
	animationLayout->addWidget(animationOn);
	animationLayout->addWidget(animationOff);
	animationGroup->setLayout(animationLayout);
	rightLayout->addWidget(animationGroup);

	// 7. 象棋的走棋步骤，每一行显示一个步骤文本
	moveHistory = new QTextEdit(this);
	moveHistory->setReadOnly(true);
	rightLayout->addWidget(moveHistory);

	// 将右边布局添加到主布局
	mainLayout->addLayout(rightLayout, 2);

	// 设置主布局
	resize(800, 600);
}
