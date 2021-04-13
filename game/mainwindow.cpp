#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QImage>
#include <QPainter>
#include <QColor>
#include <QDir>
#include <QTextStream>
#include <omp.h>
#include <QCursor>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTextCodec>
#include <QFileDialog>
#include <QtCore/qmath.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  qApp->installEventFilter(this);                       // Ловим все события

  // Работа с директориями и путями
  currentPath = QDir::currentPath();                    // Директория исполняемого файла
  currentPath.replace("/", "\\\\");                     // Заменить символы "/" на "\\"

  mapPath = "\\\\images\\\\map\\\\";                    // Путь до папки с картами относительно исполняемого файла
  mapPath.prepend(currentPath);                         // Полный путь до папки с картами
  gifPath = "\\\\images\\\\gif\\\\";                    // Путь до папки с кадрами относительно исполняемого файла
  gifPath.prepend(currentPath);                         // Полное имя папки с кадрами
  savePath = "\\\\images\\\\save\\\\";                  // Путь до папки с сохранениями относительно исполняемого файла
  savePath.prepend(currentPath);                        // Полное имя папки с сохранениями

  // Таймер
  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(timeSlot())); // Цепляем таймер к слоту
  timer->start(1000);                                        // Запуск

  // Начальные параметры Игрока
  xPlayer = 3;                   // Координаты игрока
  yPlayer = 3;                   // Координаты игрока
  aPlayer = 0;                   // Угол между осью 0X и направлением взгляда
  fPlayer = 90;                  // Угол обхвата зрением игрока
  speedPlayer = 0.1;             // Скорость передвижения
  rateOfTurn = 5;                // Скорость поворота
  postHeightDivider = 15;        // Делитель высоты столба
  offsetFromTheCeiling = 25;     // Отступ от потолка
  offsetFromTheFloor = 25;       // Отступ от пола
  imageQuality = 256;            // Качество изображения
  stepHypotenuse = 0.1;          // Шаг гипотенузы
  rangeReview = 50;              // Диапазон обзора
  widthPlayingField = 512;       // Ширина игрового поля
  heightPlayingField = 512;      // Высота игрового поля

  // Прочие начальные настройки
  ui->radioButton_imageQuality_256->setChecked(1);
  on_pushButton_clicked();
}

MainWindow::~MainWindow() { delete ui; }






// Кнопка действия
void MainWindow::on_pushButton_clicked(){
  // Получаем изображение
  image.load(mapPath + QString::number(ui->spinBox_imageNumber->value()) + ".bmp"); // Номер изображения
  w = image.width(); h = image.height(); lengthMapDiagonal = int(sqrt(pow(w,2)+pow(h,2)));
  ui->label_mapSize->setText("(" + QString::number(w) + "," + QString::number(h) + ")");

  // Получаем качество изображения
  if(ui->radioButton_imageQuality_32->isChecked())    {imageQuality = 32;   ui->spinBox_imageQuality->setValue(imageQuality);} // 32x32
  if(ui->radioButton_imageQuality_64->isChecked())    {imageQuality = 64;   ui->spinBox_imageQuality->setValue(imageQuality);} // 64x64
  if(ui->radioButton_imageQuality_128->isChecked())   {imageQuality = 128;  ui->spinBox_imageQuality->setValue(imageQuality);} // 128x128
  if(ui->radioButton_imageQuality_256->isChecked())   {imageQuality = 256;  ui->spinBox_imageQuality->setValue(imageQuality);} // 256x256
  if(ui->radioButton_imageQuality_512->isChecked())   {imageQuality = 512;  ui->spinBox_imageQuality->setValue(imageQuality);} // 512x512
  if(ui->radioButton_imageQuality_1024->isChecked())  {imageQuality = 1024; ui->spinBox_imageQuality->setValue(imageQuality);} // 1024x1024
  if(ui->radioButton_imageQuality_yours->isChecked()) {imageQuality = ui->spinBox_imageQuality->value();}                      // Своё

  // Применение разных настроек
  postHeightDivider = ui->spinBox_postHeightDivider->value();                        // Делитель высоты столба
  offsetFromTheCeiling = ui->spinBox_offsetFromTheCeiling->value();                  // Отступ от потолка
  offsetFromTheFloor = ui->spinBox_offsetFromTheFloor->value();                      // Отступ от пола
  stepHypotenuse = ui->doubleSpinBox_stepHypotenuse->value();                        // Шаг гипотенузы
  ui->doubleSpinBox_lengthBeam->setValue(obstacle[ui->spinBox_beamNumber->value()]); // Перебор длин лучей
  speedPlayer = ui->doubleSpinBox_speedPlayer->value();                              // Скорость передвижения
  rateOfTurn = ui->spinBox_rateOfTurn->value();                                      // Скорость поворота                                                                              // Скорость поворота
  fPlayer = ui->spinBox_theAngleOfWrap->value();                                     // Угол обхвата
  rangeReview = ui->spinBox_rangeReview->value();                                    // Диапазон обхвата

  shit(); // Основной алгоритм
}






// Блядское поле экспериментов
void MainWindow::shit(){

  // Получаем матрицу пикселей
  for(int i = 0; i < w; i++){
    for(int j = 0; j < h; j++){
      rgb = image.pixel(i, j);
      redBarrier[i][j] = qRed( rgb );
      greenBarrier[i][j] = qGreen( rgb );
      blueBarrier[i][j] = qBlue( rgb );
    }
  }

  //  Отрисовываем карту оригинал
  QPixmap pix_wxh(w,h);         // создаём пустой QPixmap размером с исходную картинку
  QColor pxColor_wxh;           // Хранит цвет
  QPainter p_wxh(&pix_wxh);     // Создаём объект отрисовщика и цепляем его к нашему QPixmap
  for(int i = 0; i < w; i++){
    for(int j = 0; j < h; j++){
      pxColor_wxh.setRed(redBarrier[i][j]);     // Устанавливаем красный
      pxColor_wxh.setGreen(greenBarrier[i][j]); // Устанавливаем зелёный
      pxColor_wxh.setBlue(blueBarrier[i][j]);   // Устанавливаем синий
      p_wxh.setPen(pxColor_wxh);                // Устанавливаем цвет
      p_wxh.drawPoint(i,j);                     // Закрашиваем пиксель
    }
  }
  // Установка
  ui->label_map_wxh->setPixmap(pix_wxh.scaled(256,256, Qt::KeepAspectRatio));

  // Отрисовываем карту 256x256 без игрока
  QPixmap pix_256x256(256,256);           // создаём пустой QPixmap размером с исходную картинку
  QColor pxColor_256x256;                 // Хранит цвет
  QPainter p_256x256(&pix_256x256);       // Создаём объект отрисовщика и цепляем его к нашему QPixmap
  for(int i = 0; i < w; i++){
    for(int j = 0; j < h; j++){
      for(int f = 0; f < 256/w; f++){
        for(int k = 0; k < 256/h; k++){
      pxColor_256x256.setRed(redBarrier[i][j]);         // Устанавливаем красный
      pxColor_256x256.setGreen(greenBarrier[i][j]);     // Устанавливаем зелёный
      pxColor_256x256.setBlue(blueBarrier[i][j]);       // Устанавливаем синий
      p_256x256.setPen(pxColor_256x256);                // Устанавливаем цвет
      p_256x256.drawPoint(((i*256)/w)+f,((j*256)/h)+k); // Закрашиваем пиксель
        }
      }
    }
  }

  // Сердце Рейкастинга
  int count = 0;
  for(double i = 0; i < fPlayer; i=i+fPlayer/imageQuality) {
      for (double j = 0; j < rangeReview; j=j+stepHypotenuse) {
        x = xPlayer + j*cos((aPlayer-(fPlayer/2)+i)*PI/180); // координата препятствия по x
        y = yPlayer + j*sin((aPlayer-(fPlayer/2)+i)*PI/180); // координата препятствия по y
        if (redBarrier[int(x)][int(y)]+greenBarrier[int(x)][int(y)]+blueBarrier[int(x)][int(y)] == 0) {
          obstacle[count] = j; count++; j = 100;
        }
      }
    }

  // 3D поле
  QPixmap pix_3(imageQuality,imageQuality);  // создаём пустой QPixmap размером с исходную картинку
  QColor pxColor_3;                          // Хранит цвет
  QPainter p_3(&pix_3);                      // Создаём объект отрисовщика и цепляем его к нашему QPixmap
  for(int i = 0; i < imageQuality; i++){
    for(int j = 0; j < imageQuality; j++){
      pxColor_3.setRed(230);       // Устанавливаем красный
      pxColor_3.setGreen(210);     // Устанавливаем зелёный
      pxColor_3.setBlue(210);      // Устанавливаем синий
      p_3.setPen(pxColor_3);       // Устанавливаем цвет
      p_3.drawPoint(i,j);          // Закрашиваем пиксель
    }
  }
  for(int i = 0; i < imageQuality; i++){
    for(int j = int(obstacle[i]*(imageQuality/postHeightDivider)+(offsetFromTheCeiling))/2;
         j < imageQuality-int(obstacle[i]*(imageQuality/postHeightDivider)+(offsetFromTheFloor))/2;
         j++)
    {
      pxColor_3.setRed(255-int(obstacle[i]*30));     // Устанавливаем красный
      pxColor_3.setGreen(0);                         // Устанавливаем зелёный
      pxColor_3.setBlue(0);                          // Устанавливаем синий
      p_3.setPen(pxColor_3);                         // Устанавливаем цвет
      p_3.drawPoint(i,j);                            // Закрашиваем пиксель
    }
  }

  // Установка изображения 3D
  ui->label_pix_512x512->setPixmap(pix_3.scaled(widthPlayingField, heightPlayingField, Qt::KeepAspectRatio));

  // Отрисовка лучей на карте 256x256
  QPen pen_256x256(QRgb(0x8e3975));
  pen_256x256.setWidth(2);
  p_256x256.setPen(pen_256x256);
  // Отрисовка центрального луча
  p_256x256.drawLine(int((xPlayer*256)/w),
                     int((yPlayer*256)/h),
                     int(((xPlayer*256)/w)+1000*cos(aPlayer*PI/180)),
                     int(((yPlayer*256)/h)+1000*sin(aPlayer*PI/180)));
  // Отрисовка луча левой границы обзора
  p_256x256.drawLine(int((xPlayer*256)/w),
                     int((yPlayer*256)/h),
                     int(((xPlayer*256)/w)+1000*cos((aPlayer-fPlayer/2)*PI/180)),
                     int(((yPlayer*256)/h)+1000*sin((aPlayer-fPlayer/2)*PI/180)));
  // Отрисовка луча правой границы обзора
  p_256x256.drawLine(int((xPlayer*256)/w),
                     int((yPlayer*256)/h),
                     int(((xPlayer*256)/w)+1000*cos((aPlayer+fPlayer/2)*PI/180)),
                     int(((yPlayer*256)/h)+1000*sin((aPlayer+fPlayer/2)*PI/180)));

  // Установка карты 256x256
  ui->label_map_256x256->setPixmap(pix_256x256.scaled(256,256, Qt::KeepAspectRatio));
}






// Все события приходят сюда
// (вычисляем координаты)
#pragma warning(default:4716)
bool MainWindow::eventFilter(QObject *watched, QEvent *event){
  // Глобальные координаты окна
  xGCW = (this)->geometry().x(); // Координаты left-top главного окна приложения
  yGCW = (this)->geometry().y(); // Координаты left-top главного окна приложения
  resizeW = (this)->width();     // Ширина окна
  resizeH = (this)->height();    // Высота окна
//  widthPlayingField = ui->label_pix_512x512->width();   // Ширина игрового поля
//  heightPlayingField = ui->label_pix_512x512->height(); // Высота игрового поля

  // Отслеживаем события мыши
  if(event->type() == QEvent::MouseMove){
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    xC = mouseEvent->x();        // Координаты курсора на теле виджета
    yC = mouseEvent->y();        // Координаты курсора на теле виджета
    xGC = mouseEvent->globalX(); // Глобальные координаты курсора
    yGC = mouseEvent->globalY(); // Глобальные координаты курсора
  }

  // Вывод Координат Курсора На Теле Виджета
  ui->label_xyC->setText("(" + QString::number(xC)   + "," + QString::number(yC)   + ")");
  // Вывод Глобальных Координат Курсора
  ui->label_xyGC->setText("(" + QString::number(xGC)   + "," + QString::number(yGC)   + ")");
  // Вывод Глобальных Координат Окна
  ui->label_xyGCW->setText("(" + QString::number(xGCW)   + "," + QString::number(yGCW)   + ")");
  // Вывод Размеров Окна
  ui->label_resizeXYGCW->setText("(" + QString::number(resizeW)   + "," + QString::number(resizeH)   + ")");
  // Вывод Размеров игрового Поля
  ui->label_sizePlayingField->setText("(" + QString::number(widthPlayingField) + "," + QString::number(heightPlayingField) + ")");

  QObject::eventFilter(watched, event);
}



// Обработка событий нажатия клавиш
void MainWindow::keyPressEvent(QKeyEvent *keyEvent) {
  QTextCodec *russiancodec = QTextCodec::codecForName("Cp1251");
  QTextCodec::setCodecForLocale (russiancodec);

  if(keyEvent->text() == 'a') {
    if(1)
    {
      xPlayer = xPlayer + speedPlayer*cos((aPlayer-90)*PI/180); // координата игрока по x
      yPlayer = yPlayer + speedPlayer*sin((aPlayer-90)*PI/180); // координата игрока по y
    }
  }
  if(keyEvent->text() == 'w') {
    if(1)
    {
      xPlayer = xPlayer + speedPlayer*cos((aPlayer-0)*PI/180); // координата игрока по x
      yPlayer = yPlayer + speedPlayer*sin((aPlayer-0)*PI/180); // координата игрока по y
    }
  }
  if(keyEvent->text() == 'd') {
    if(1)
    {
      xPlayer = xPlayer + speedPlayer*cos((aPlayer+90)*PI/180); // координата игрока по x
      yPlayer = yPlayer + speedPlayer*sin((aPlayer+90)*PI/180); // координата игрока по y
    }
  }
  if(keyEvent->text() == 's') {
    if(1)
    {
      xPlayer = xPlayer + speedPlayer*cos((aPlayer-180)*PI/180); // координата игрока по x
      yPlayer = yPlayer + speedPlayer*sin((aPlayer-180)*PI/180); // координата игрока по y
    }
  }

  if(keyEvent->text() == 'j') {aPlayer = aPlayer - rateOfTurn;}
  if(keyEvent->text() == 'l') {aPlayer = aPlayer + rateOfTurn;}

  if(xPlayer > w) xPlayer = xPlayer - w; if(xPlayer < 0) xPlayer = xPlayer + w;
  if(yPlayer > h) yPlayer = yPlayer - h; if(yPlayer < 0) yPlayer = yPlayer + h;
  if(aPlayer > 360) aPlayer = aPlayer - 360; if(aPlayer < 0) aPlayer = aPlayer + 360;

  ui->label_key->setText(keyEvent->text());                                                                             // Вывод нажатой клавиши
  ui->label_xyPlayer->setText("(" + QString::number(xPlayer, 'g', 5) + " , " + QString::number(yPlayer, 'g', 5) + ")"); // Вывод координат Игрока
  ui->label_corner->setText(QString::number(aPlayer));                                                                  // Вывод угла

  shit(); // Основной алгоритм
}









// Таймер
void MainWindow::timeSlot(){

}
// Перебор длин лучей
void MainWindow::on_spinBox_beamNumber_valueChanged(int arg1){
  ui->doubleSpinBox_lengthBeam->setValue(obstacle[arg1]);
}
// Обработка изменения Скорости передвижения
void MainWindow::on_doubleSpinBox_speedPlayer_valueChanged(double arg1){
  speedPlayer = arg1;
  shit(); // Основной алгоритм
}
// Обработка изменения скорости поворота
void MainWindow::on_spinBox_rateOfTurn_valueChanged(int arg1){
  rateOfTurn = arg1;
  shit(); // Основной алгоритм
}
// Обработка изменения угла обхвата
void MainWindow::on_spinBox_theAngleOfWrap_valueChanged(int arg1){
  fPlayer = arg1;
  shit(); // Основной алгоритм
}
