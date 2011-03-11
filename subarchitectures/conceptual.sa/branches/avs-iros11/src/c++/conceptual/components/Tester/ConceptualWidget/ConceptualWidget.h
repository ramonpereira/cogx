/**
 * @author Andrzej Pronobis
 *
 * Main dialog.
 */

#ifndef CONCEPTUALWIDGET_H
#define CONCEPTUALWIDGET_H

// Conceptual.SA
#include "ui_ConceptualWidget.h"
#include "ConceptualData.hpp"

// Qt&Std
#include <QtGui/QDialog>
#include <queue>

class QTimer;

namespace conceptual
{
	class Tester;
}

class ObjectPlacePropertyDialog;
class ObjectSearchResultDialog;
class RCVisualizer;

class ConceptualWidget : public QWidget, public Ui::ConceptualWidgetClass
{
    Q_OBJECT

    friend class ObjectPlacePropertyDialog;
    friend class ObjectSearchResultDialog;
    friend class RCVisualizer;

	struct Event
	{
		int curRoomId;
		int curPlaceId;
		std::vector<int> curRoomPlaces;
		std::vector<ConceptualData::EventInfo> infos;
		std::vector<double> curRoomCategories;
		std::vector<double> curShapes;
		std::vector<double> curAppearances;
		std::vector<double> curObjects;
	};


public:
    ConceptualWidget(QWidget *parent, conceptual::Tester *component);
    ~ConceptualWidget();

public:

    void newWorldState(ConceptualData::WorldStatePtr wsPtr);


private slots:

	void categoriesButtonClicked();
	void objectsButtonClicked();
	void sendQueryButtonClicked();
	void refreshVarsButtonClicked();
	void refreshWsButtonClicked();
	void showGraphButtonClicked();
	void visualizeButtonClicked();
	void varListCurrentTextChanged(const QString &curText);
	void factorListCurrentTextChanged(const QString &curText);
	void wsTimerTimeout();
	void posTimerTimeout();
	void addObjectPlacePropertyButtonClicked();
	void addObjectSearchResultButtonClicked();
	void addEvent(Event event);


private:

	int getRoomForPlace(ConceptualData::WorldStatePtr wsPtr, int placeId);
	void collectEventInfo(Event event);
	void getPlacesForRoom(ConceptualData::WorldStatePtr wsPtr, int roomId, std::vector<int> &places);
	double getExistsProbability(SpatialProbabilities::ProbabilityDistribution &probDist);


private:
    conceptual::Tester *_component;

    QTimer *_wsTimer;
    QTimer *_posTimer;
    int _wsCount;

    std::queue<qint64> _wsUpdateTimes;

	pthread_mutex_t _worldStateMutex;
	pthread_mutex_t _eventsMutex;
	ConceptualData::WorldStatePtr _wsPtr;

	bool _collect;
	int _prevPlace;
	long _eventNo;

	std::vector<Event> _events;

};



#endif // ConceptualWidget_H
