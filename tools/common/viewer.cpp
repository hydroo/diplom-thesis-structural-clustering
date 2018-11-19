#include "common/viewer.hpp"

#include <QDialog>
//#include <QLabel>
#include <QBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
//#include <QScrollArea>
#include <QWheelEvent>

struct Viewer;

int g_dialogCount = 0;
QSet<Viewer*> g_viewers;

class Viewer : public QDialog {
	Q_OBJECT

	class GraphicsView : public QGraphicsView {
	public:
		GraphicsView(QGraphicsScene* scene, QWidget* parent = nullptr) : QGraphicsView(scene, parent) {}
	protected:
		virtual void wheelEvent (QWheelEvent* e) {
			if ((e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier && e->angleDelta().x() == 0) {

				QPoint  pos  = e->pos();
				QPointF posf = this->mapToScene(pos);

				double by;
				double angle = e->angleDelta().y();

				if      (angle > 0) { by = 1 + (angle / 360 * 0.1); }
				else if (angle < 0) { by = 1 - (-angle / 360 * 0.1); }
				else                { by = 1; }

				this->scale(by, by);

				double w = this->viewport()->width();
				double h = this->viewport()->height();

				double wf = this->mapToScene(QPoint(w-1, 0)).x() - this->mapToScene(QPoint(0,0)).x();
				double hf = this->mapToScene(QPoint(0, h-1)).y() - this->mapToScene(QPoint(0,0)).y();

				double lf = posf.x() - pos.x() * wf / w;
				double tf = posf.y() - pos.y() * hf / h;

				/* try to set viewport properly */
				this->ensureVisible(lf, tf, wf, hf, 0, 0);

				QPointF newPos = this->mapToScene(pos);

				/* readjust according to the still remaining offset/drift
				 * I don't know how to do this any other way */
				this->ensureVisible(QRectF(QPointF(lf, tf) - newPos + posf, QSizeF(wf, hf)), 0, 0);

				e->accept();
			}

			if ((e->modifiers() & Qt::ControlModifier) != Qt::ControlModifier) {
				/* no scrolling while control is held */
				QGraphicsView::wheelEvent(e);
			}
		}
	};

private:
	int margin = 38; // chosen, scrollbar and window frame, qt/X11 has no easy way to find this out
public:
	Viewer(QWidget* parent = nullptr) : QDialog(parent) {}

	//void init(const QImage& image, const QString& title) {

	//	QLabel* imageLabel = new QLabel(this);
	//	imageLabel->setPixmap(QPixmap::fromImage(image));
	//	imageLabel->resize(imageLabel->pixmap()->size());

	//	QScrollArea* scrollArea = new QScrollArea(this);
	//	scrollArea->setBackgroundRole(QPalette::Dark);
	//	scrollArea->setWidget(imageLabel);

	//	scrollArea->resize(QSize(imageLabel->size().width() + margin, imageLabel->size().height() + margin));

	//	init(scrollArea, title);
	//}
	//
	void init(QGraphicsScene* s, const QString& title) {
		GraphicsView *v = new GraphicsView(s, this);
		v->setRenderHint(QPainter::Antialiasing);
		s->setParent(v);
		v->resize(s->sceneRect().size().toSize() + QSize(margin, margin));
		init((QWidget*) v, title);
	}

	/* reparents 'w' */
	void init(QWidget *w, const QString& title) {
		this->setWindowTitle(title);

		QBoxLayout* boxLayout = new QBoxLayout(QBoxLayout::LeftToRight, this);
		boxLayout->addWidget(w);
		w->setParent(this);
		this->setLayout(boxLayout);

		this->resize(w->size());

		connect(this, SIGNAL(accepted()), this, SLOT(exit()));
		connect(this, SIGNAL(rejected()), this, SLOT(exit()));
	}
private slots:
	void exit() {
		assert(g_dialogCount > 0);
		g_dialogCount -= 1;
	}
};

//void Viewer_show(const QImage& i, const QString& title) {
//	auto* v = new Viewer;
//
//	v->init(i, title);
//
//	g_dialogCount += 1;
//	g_viewers.insert(v);
//
//	v->show();
//}
//
void Viewer_show(QGraphicsScene* s, const QString& title) {
	auto* v = new Viewer;

	v->init(s, title);

	g_dialogCount += 1;
	g_viewers.insert(v);

	v->show();
}

void Viewer_wait() {
	while (g_dialogCount > 0) {
		QCoreApplication::processEvents();
		QThread::msleep(20);
	}

	foreach (auto* v, g_viewers) {
		delete v;
	}
	g_viewers = QSet<Viewer*>();
}

#include "viewer.moc"
