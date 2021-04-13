#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QVector>
#include <array>

#include <QGLWidget>
#include <QtOpenGL>

#define PI 3.14159265

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  // Работа с директориями
  QString currentPath;         // Хранит полное имя каталога исполняемого файла
  char currentPath_char[256];  //
  QString img;                 // Хранит полное имя обрабатываемого изображения
  char img_char[256];          //
  QString gifPath;             // Хранит полное имя каталога с gif
  QString mapPath;             // Хранит полное имя каталога с картами
  QString savePath;            // Хранит полное имя каталога c сохранениями
  QString nMap_str;            // Хранит номер карты

  QTimer *timer;               // QTimer

  QString str;                 // Строка вывода

  int xC, yC;                  // Координаты курсора на теле виджета
  int xGC, yGC;                // Глобальные координаты курсора
  int xGCW, yGCW;              // Координаты left-top главного окна приложения
  int resizeW, resizeH;        // Ширина и Высота окна окна

  QImage imgMap[256];          // Кол-во карт
  QImage image;                // Сюда загрузим изображение
  QRgb rgb;                    // Цвет

  int w;                           // Ширина изображения
  int h;                           // Высота изображения
  int widthPlayingField;      // Ширина игрового поля
  int heightPlayingField;     // Высота игрового поля
  int lengthMapDiagonal;      // Длина диагонали карты
  int rangeReview;                 // Диапазон обзора
  int imageQuality;                // Качество изображения

  // Карта препятствий
  int redBarrier[64][64];
  int greenBarrier[64][64];
  int blueBarrier[64][64];

  double xPlayer, yPlayer;  // Координаты игрока
  int aPlayer;              // Угол между OX и направлением обзора игрока
  double fPlayer;           // Угол обхвата зрением игрока
  double a,b,c;             // Прямоугольный треугольник
  double x,y;               // Координаты текущего препятствия

  double obstacle[1024];    // Массив хранит расстояние до препятствий в поле зрении

  double speedPlayer;       // Скорость передвижения
  int rateOfTurn;           // Скорость поворота
  int postHeightDivider;    // Делитель высоты столба
  int offsetFromTheCeiling; // Отступ от потолка
  int offsetFromTheFloor;   // Отступ от пола
  double stepHypotenuse;    // Шаг гипотенузы

protected:
  bool eventFilter(QObject *watched, QEvent *event);             // Ловим события
  void keyPressEvent(QKeyEvent *keyEvent);                       // Ловим события нажатия клавиш
private slots:
  void timeSlot();                                               // Таймер
  void shit();                                                   // основной кусок кода
  void on_pushButton_clicked();                                  // Действие                     
  void on_doubleSpinBox_speedPlayer_valueChanged(double arg1); // Изменение значения скорости
  void on_spinBox_rateOfTurn_valueChanged(int arg1);             // Скорость поворота
  void on_spinBox_theAngleOfWrap_valueChanged(int arg1);
  void on_spinBox_beamNumber_valueChanged(int arg1);             // Перебор лучей

private:
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
