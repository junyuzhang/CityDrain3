#include "simulationscene.h"
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QWidget>
#include <QTreeWidget>
#include <QDebug>

#include "newsimulationdialog.h"

#include <noderegistry.h>
#include <simulationregistry.h>
#include <simulation.h>
#include <node.h>
#include <nodeitem.h>
#include <portitem.h>
#include <mapbasedmodel.h>

#include <boost/lexical_cast.hpp>


using namespace std;

SimulationScene::SimulationScene(QObject *parent)
	: QGraphicsScene(parent) {
	model = new MapBasedModel();
	node_reg = new NodeRegistry();
	sim_reg = new SimulationRegistry();
	simulation = 0;
	current_connection = 0;
}

SimulationScene::~SimulationScene() {
	delete(model);
}

void SimulationScene::dropEvent(QGraphicsSceneDragDropEvent *event) {
	event->accept();
	QTreeWidget *treeWidget = (QTreeWidget*) event->source();
	string klassName = treeWidget->selectedItems()[0]->text(0).toStdString();
	QGraphicsScene::dragMoveEvent(event);

	Node *node = node_reg->createNode(klassName);
	//default id is klassname_+counter
	if (!ids.contains(klassName))
		ids[klassName] = 0;
	string id_count = lexical_cast<string>(ids[node->getClassName()]++);
	node->setId(node->getClassName() + string("_") + id_count);
	model->addNode(node);
	NodeItem *nitem = new NodeItem(node);
	this->addItem(nitem);
	nitem->setPos(event->scenePos());
	node_items << nitem;
}

void SimulationScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event) {
	event->accept();
}

void SimulationScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	connection_start = (PortItem *) itemAt(event->scenePos());

	if (connection_start && isOutPort(connection_start) && !connection_start->isConnected()) {
		QLineF l(connection_start->scenePos() + connection_start->boundingRect().center(),
				 event->scenePos());
		current_connection = addLine(l);
		current_connection->setZValue(0);
		return;
	}

	connection_start = 0;
	QGraphicsScene::mousePressEvent(event);
}

void SimulationScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	if (connection_start) {
		QLineF f = current_connection->line();
		f.setP2(event->scenePos());
		current_connection->setLine(f);
		update();
		return;
	}
	QGraphicsScene::mouseMoveEvent(event);
	return;
}

void SimulationScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	PortItem *connection_end = (PortItem *) itemAt(event->scenePos());

	if (connection_end && isInPort(connection_end) && !connection_end->isConnected()) {
		if (!newSimulation()) {
			connection_start = 0;
			delete current_connection;
			return;
		}

		QLineF f = current_connection->line();
		f.setP2(connection_end->scenePos()+connection_end->boundingRect().center());
		current_connection->setLine(f);
		update();

		connection_start->setSourceOf(current_connection);
		connection_end->setSinkOf(current_connection);

		Node *start = connection_start->getNodeItem()->getNode();
		Node *end = connection_end->getNodeItem()->getNode();
		string out_port = connection_start->getPortName().toStdString();
		string in_port = connection_end->getPortName().toStdString();

		NodeConnection *con = simulation->createConnection(start, out_port, end, in_port);
		model->addConnection(con);
//TODO	current_connection->setData();
		connection_start = 0;
		current_connection = 0;
		return;
	}


	if (current_connection)
		delete current_connection;
	current_connection = 0;
	connection_start = 0;
	QGraphicsScene::mouseReleaseEvent(event);
}

bool SimulationScene::newSimulation() {
	if (simulation)
		return true;
	NewSimulationDialog ns(sim_reg);
	if (ns.exec()) {
		simulation = ns.createSimulation();
		simulation->setModel(model);
		Q_EMIT(simulationCreated());
		return true;
	}

	return false;
}

bool SimulationScene::isInPort(QGraphicsItem *item) const {
	Q_FOREACH(NodeItem *node, node_items) {
		if (node->in_ports.contains((PortItem*) item)) {
			return true;
		}
	}
	return false;
}

bool SimulationScene::isOutPort(QGraphicsItem *item) const {
	Q_FOREACH(NodeItem *node, node_items) {
		if (node->out_ports.contains((PortItem*) item)) {
			return true;
		}
	}
	return false;
}
