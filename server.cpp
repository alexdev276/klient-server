#include "server.h"
#include "serversocketadapter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTextStream>
#include <QtMath>
#include <algorithm>

Server::Server(int tcpPort, int udpPort, QWidget *parent)
    : QWidget(parent),
    m_ptcpServer(new QTcpServer(this)),
    m_udpAdapter(new UdpAdapter(this, udpPort)),
    m_nextClientId(1),
    m_tcpPort(tcpPort),
    m_udpPort(udpPort),
    m_isCalculating(false),
    m_totalTasks(0), m_completedTasks(0),
    m_finalArea(0.0)
{
    setWindowTitle("Сервер");
    resize(760, 520);

    // ---------- UI ----------
    m_taskCombo = new QComboBox(this);
    m_taskCombo->addItem("Вариант 1: площадь sin(x) при Y>0", 1);
    m_taskCombo->addItem("Вариант 4: корни cos³(x)", 4);

    m_aSpin = new QDoubleSpinBox(this);
    m_aSpin->setRange(-1e15, 1e15);
    m_aSpin->setDecimals(12);
    m_aSpin->setValue(0.0);

    m_bSpin = new QDoubleSpinBox(this);
    m_bSpin->setRange(-1e15, 1e15);
    m_bSpin->setDecimals(12);
    m_bSpin->setValue(4.0 * M_PI);

    m_dxSpin = new QDoubleSpinBox(this);
    m_dxSpin->setRange(1e-15, 1e9);      // <-- очень маленький минимум
    m_dxSpin->setDecimals(15);           // <-- до 15 знаков после запятой
    m_dxSpin->setSingleStep(1e-6);
    m_dxSpin->setValue(0.01);

    m_startBtn = new QPushButton("Запустить", this);

    QFormLayout *form = new QFormLayout;
    form->addRow("Задание:",              m_taskCombo);
    form->addRow("Начало диапазона (A):", m_aSpin);
    form->addRow("Конец диапазона (B):",  m_bSpin);
    form->addRow("Шаг dx (точность):",    m_dxSpin);

    QHBoxLayout *topRow = new QHBoxLayout;
    topRow->addLayout(form, 1);
    topRow->addWidget(m_startBtn);

    QGroupBox *ctrlBox = new QGroupBox("Параметры задачи", this);
    ctrlBox->setLayout(topRow);

    m_statusLabel = new QLabel("Подключено клиентов: 0 | Ожидание запуска", this);

    m_logView = new QPlainTextEdit(this);
    m_logView->setReadOnly(true);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(ctrlBox);
    main->addWidget(m_statusLabel);
    main->addWidget(m_logView, 1);

    connect(m_startBtn, &QPushButton::clicked, this, &Server::onStartClicked);

    // ---------- Network ----------
    connect(m_ptcpServer, &QTcpServer::newConnection, this, &Server::on_newConnection);
    connect(m_udpAdapter, &UdpAdapter::message,       this, &Server::on_udpMessage);

    if (!m_ptcpServer->listen(QHostAddress::Any, tcpPort)) {
        QString err = m_ptcpServer->errorString();
        m_ptcpServer->close();
        log(QString("[ОШИБКА] Не удалось запустить TCP сервер: %1").arg(err));
    } else {
        log("========================================");
        log("=== СЕРВЕР ЗАПУЩЕН ===");
        log(QString("TCP порт: %1").arg(tcpPort));
        log(QString("UDP порт: %1").arg(udpPort));
        log("========================================");
    }
    updateStatus();
}

Server::~Server()
{
    qDeleteAll(m_clients);
    m_clients.clear();
    m_clientIds.clear();
}

void Server::log(const QString &msg)
{
    QTextStream(stdout) << msg << "\n";
    if (m_logView) m_logView->appendPlainText(msg);
}

void Server::updateStatus()
{
    QString state = m_isCalculating
                        ? QString("Вычисление: %1/%2")
                              .arg(m_completedTasks).arg(m_totalTasks)
                        : "Ожидание запуска";
    m_statusLabel->setText(QString("Подключено клиентов: %1 | %2")
                               .arg(m_clients.size()).arg(state));
}

int Server::clientId(ISocketAdapter *sock) const
{
    return m_clientIds.value(sock, -1);
}

void Server::sendTo(ISocketAdapter *sock, const QString &msg)
{
    if (!sock) return;
    sock->sendString(msg + "\n");
}

void Server::broadcast(const QString &msg)
{
    foreach (ISocketAdapter *sock, m_clients)
    sendTo(sock, msg);
}

void Server::on_newConnection()
{
    QTcpSocket *pClientSock = m_ptcpServer->nextPendingConnection();
    ISocketAdapter *pSockHandle = new ServerSocketAdapter(pClientSock, this);
    m_clients.push_back(pSockHandle);

    int id = m_nextClientId++;
    m_clientIds.insert(pSockHandle, id);

    QString peer = QString("%1:%2")
                       .arg(pClientSock->peerAddress().toString())
                       .arg(pClientSock->peerPort());
    log(QString("[СЕРВЕР] Клиент #%1 подключился (%2). Всего клиентов: %3")
            .arg(id).arg(peer).arg(m_clients.size()));

    sendTo(pSockHandle, QString("WELCOME:%1").arg(id));

    connect(pSockHandle, &ISocketAdapter::disconnected,
            this, &Server::on_disconnected);
    connect(pSockHandle, &ISocketAdapter::message,
            this, &Server::on_message);

    updateStatus();
}

void Server::on_disconnected()
{
    ISocketAdapter *client = qobject_cast<ISocketAdapter*>(sender());
    if (!client) return;

    int id = clientId(client);
    log(QString("[СЕРВЕР] Клиент #%1 отключился.").arg(id));

    m_clients.removeOne(client);
    m_clientIds.remove(client);
    client->deleteLater();
    updateStatus();
}

void Server::on_udpMessage(QString msg)
{
    log(QString("[СЕРВЕР UDP] Получено: %1").arg(msg));
    if (msg.startsWith("UDP_BROADCAST:"))
        broadcast(msg.mid(14));
}

void Server::onStartClicked()
{
    if (m_clients.isEmpty()) {
        log("[СЕРВЕР] Нельзя запустить: нет подключённых клиентов.");
        return;
    }

    int    type = m_taskCombo->currentData().toInt();
    double A    = m_aSpin->value();
    double B    = m_bSpin->value();
    double dx   = m_dxSpin->value();

    if (B <= A) {
        log("[СЕРВЕР] Ошибка: B должно быть больше A.");
        return;
    }
    if (dx <= 0.0) {
        log("[СЕРВЕР] Ошибка: dx должно быть > 0.");
        return;
    }

    // qint64 — без переполнения даже при 1e10+ шагов
    qint64 steps = static_cast<qint64>((B - A) / dx);
    if (steps < 1) steps = 1;

    if (m_isCalculating) {
        log("[СЕРВЕР] Смена задания: прерываю текущее вычисление.");
        m_isCalculating = false;
    }

    log("========================================");
    log(QString("[СЕРВЕР] Запуск: задание=%1, A=%2, B=%3, dx=%4, шагов=%5")
            .arg(type).arg(A, 0, 'g', 15).arg(B, 0, 'g', 15)
            .arg(dx, 0, 'g', 15).arg(steps));
    distributeTasks(type, A, B, steps);
}

void Server::on_message(QString msg)
{
    int id = clientId(qobject_cast<ISocketAdapter*>(sender()));

    if (msg.startsWith("RESULT:")) {
        if (!m_isCalculating) return;

        QStringList parts = msg.split(":");
        if (parts.size() < 4) return;

        int    type   = parts[1].toInt();
        qint64 taskId = parts[2].toLongLong();  // <-- qint64

        m_completedTasks++;

        if (type == 1) {
            double val = parts[3].toDouble();
            m_finalArea += val;

            log(QString("[СЕРВЕР] Клиент #%1 выполнил шаг %2 (площадь=%3) | %4/%5")
                    .arg(id).arg(taskId)
                    .arg(val, 0, 'f', 8)
                    .arg(m_completedTasks).arg(m_totalTasks));

            if (m_completedTasks >= m_totalTasks) {
                m_isCalculating = false;
                QString finalMsg = QString("FINAL:1:%1")
                                       .arg(m_finalArea, 0, 'f', 8);
                log(QString("[СЕРВЕР] === ИТОГ: площадь = %1 ===")
                        .arg(m_finalArea, 0, 'f', 8));
                broadcast(finalMsg);
            }
        }
        else if (type == 4) {
            QString rootsStr = parts[3];
            int found = 0;
            if (!rootsStr.isEmpty()) {
                QStringList list = rootsStr.split(",");
                found = list.size();
                for (const QString &s : list)
                    m_allRoots.append(s.toDouble());
            }

            log(QString("[СЕРВЕР] Клиент #%1 выполнил шаг %2 (корней: %3) | %4/%5")
                    .arg(id).arg(taskId).arg(found)
                    .arg(m_completedTasks).arg(m_totalTasks));

            if (m_completedTasks >= m_totalTasks) {
                m_isCalculating = false;
                std::sort(m_allRoots.begin(), m_allRoots.end());
                QList<double> uniqueRoots;
                for (double r : m_allRoots) {
                    if (uniqueRoots.isEmpty() ||
                        qAbs(r - uniqueRoots.last()) > 1e-4)
                        uniqueRoots.append(r);
                }
                QString rootsOut;
                for (double r : uniqueRoots)
                    rootsOut += QString::number(r, 'f', 6) + ", ";
                if (!rootsOut.isEmpty()) rootsOut.chop(2);

                QString finalMsg = QString("FINAL:4:%1").arg(rootsOut);
                log(QString("[СЕРВЕР] === ИТОГ: корни: %1 ===").arg(rootsOut));
                broadcast(finalMsg);
            }
        }
        updateStatus();
    }
    else if (msg.startsWith("START_CALC:")) {
        broadcast("SERVER: Параметры задаются в окне сервера.");
    }
    else {
        log(QString("[СЕРВЕР] Клиент #%1 (чат): %2").arg(id).arg(msg));
        broadcast(msg);
    }
}

void Server::distributeTasks(int type, double A, double B, qint64 steps)
{
    if (m_clients.isEmpty()) {
        log("[СЕРВЕР] Нет клиентов!");
        return;
    }

    m_isCalculating  = true;
    m_totalTasks     = steps;
    m_completedTasks = 0;
    m_finalArea      = 0.0;
    m_allRoots.clear();
    updateStatus();

    double dx = (B - A) / static_cast<double>(steps);
    int clientIdx = 0;

    log(QString("[СЕРВЕР] Круговое распределение %1 задач между %2 клиентами...")
            .arg(steps).arg(m_clients.size()));

    for (qint64 i = 0; i < steps; ++i) {
        double a = A + i * dx;
        double b = A + (i + 1) * dx;

        ISocketAdapter *sock = m_clients[clientIdx];
        int id = clientId(sock);

        QString task = QString("TASK:%1:%2:%3:%4")
                           .arg(type).arg(i)
                           .arg(a, 0, 'g', 17)
                           .arg(b, 0, 'g', 17);

        // Логируем каждый шаг — если шагов очень много, это может «залить» лог.
        // Раскомментируйте проверку ниже, если хотите логировать только первые N:
        // if (i < 1000) {
        log(QString("[СЕРВЕР] Клиент #%1 - шаг %2  [%3 ; %4]")
                .arg(id).arg(i)
                .arg(a, 0, 'f', 8)
                .arg(b, 0, 'f', 8));
        // }

        sendTo(sock, task);
        clientIdx = (clientIdx + 1) % m_clients.size();
    }

    log("[СЕРВЕР] Все задачи отправлены, ждём результаты...");
    m_startBtn->setText("Сменить задание");
}