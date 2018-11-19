#ifndef IMAGE_VIEWER_HPP
#define IMAGE_VIEWER_HPP

#include "common/prereqs.hpp"

//#include <QImage>
struct QGraphicsScene;

//void Viewer_show(const QImage& i, const QString& title);
/* reparents 's'. no deletion required after calling Viewer_show */
void Viewer_show(QGraphicsScene* s, const QString& title);

void Viewer_wait();

#endif /* IMAGE_VIEWER_HPP */
