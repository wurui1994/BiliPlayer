#include "Common.h"
#include "Prefer.h"
#include "Setting.h"
#include "Application.h"
#include "Utils.h"
#include "Danmaku.h"
#include "ARender.h"


void Prefer::setupPlay()
{
	//Playing
	{
		widgetPlaying = new QWidget(this);
		auto list = new QVBoxLayout(widgetPlaying);
		auto c = new QGridLayout;
		QCheckBox* load = new QCheckBox(tr("clear when reloading"), widgetPlaying);
		load->setChecked(Setting::getValue("/Playing/Clear", false));
		connect(load, &QCheckBox::stateChanged, [](int state)
		{
			Setting::setValue("/Playing/Clear", state == Qt::Checked);
		});
		c->addWidget(load, 0, 0);

		QCheckBox* load1 = new QCheckBox(tr("auto delay after loaded"), widgetPlaying);
		load1->setChecked(Setting::getValue("/Playing/Delay", false));
		connect(load1, &QCheckBox::stateChanged, [](int state)
		{
			Setting::setValue("/Playing/Delay", state == Qt::Checked);
		});
		c->addWidget(load1, 0, 1);

		QCheckBox* load2 = new QCheckBox(tr("load local subtitles"), widgetPlaying);
		load2->setChecked(Setting::getValue("/Playing/Subtitle", true));
		connect(load2, &QCheckBox::stateChanged, [](int state)
		{
			Setting::setValue("/Playing/Subtitle", state == Qt::Checked);
		});
		c->addWidget(load2, 1, 0);

		QCheckBox* load3 = new QCheckBox(tr("auto play after loaded"), widgetPlaying);
		load3->setChecked(Setting::getValue("/Playing/Immediate", false));
		connect(load3, &QCheckBox::stateChanged, [](int state)
		{
			Setting::setValue("/Playing/Immediate", state == Qt::Checked);
		});
		c->addWidget(load3, 1, 1);

		QGroupBox* box = new QGroupBox(tr("loading"), widgetPlaying);
		box->setLayout(c);
		list->addWidget(box);

		auto s = new QHBoxLayout;
		QLineEdit* play = new QLineEdit(widgetPlaying);
		play->setText(Setting::getValue("/Danmaku/Speed", QString("125+%{width}/5")));
		connect(play, &QLineEdit::editingFinished, [=]()
		{
			Setting::setValue("/Danmaku/Speed", play->text());
		});
		s->addWidget(play);

		QGroupBox* box2 = new QGroupBox(tr("slide speed"), widgetPlaying);
		box2->setToolTip(tr("%{width} means the width of an danmaku"));
		box2->setLayout(s);
		list->addWidget(box2);

		auto l = new QHBoxLayout;
		QLineEdit* play1 = new QLineEdit(widgetPlaying);
		play1->setText(Setting::getValue("/Danmaku/Life", QString("5")));
		connect(play1, &QLineEdit::editingFinished, [=]()
		{
			Setting::setValue("/Danmaku/Life", play1->text());
		});
		l->addWidget(play1);

		QGroupBox* box3 = new QGroupBox(tr("life time"), widgetPlaying);
		box3->setToolTip(tr("%{width} means the width of an danmaku"));
		box3->setLayout(l);
		list->addWidget(box3);

		auto e = new QHBoxLayout;
		int state = Setting::getValue("/Danmaku/Scale/Fitted", 0x1);
		QCheckBox* fitted = new QCheckBox(tr("ordinary"), widgetPlaying);
		fitted->setChecked((state & 0x2) > 0);
		QCheckBox* fitted1 = new QCheckBox(tr("advanced"), widgetPlaying);
		fitted1->setChecked((state & 0x1) > 0);
		auto slot = [=]()
		{
			int n = fitted->checkState() == Qt::Checked;
			int a = fitted1->checkState() == Qt::Checked;
			Setting::setValue("/Danmaku/Scale/Fitted", (n << 1) + a);
		};
		connect(fitted, &QCheckBox::stateChanged, slot);
		connect(fitted1, &QCheckBox::stateChanged, slot);
		e->addWidget(fitted, 1);
		e->addWidget(fitted1, 1);

		QGroupBox* box4 = new QGroupBox(tr("scale to fitted"), widgetPlaying);
		box4->setLayout(e);

		auto a = new QHBoxLayout;
		factor = new QLineEdit(widgetPlaying);
		factor->setText(QString::number(Setting::getValue("/Danmaku/Scale/Factor", 1.0), 'f', 2));
		connect(factor, &QLineEdit::editingFinished, [=]()
		{
			Setting::setValue("/Danmaku/Scale/Factor", factor->text().toDouble());
		});
		a->addWidget(factor);

		QGroupBox* box5 = new QGroupBox(tr("scale by factor"), widgetPlaying);
		box5->setLayout(a);

		auto o = new QHBoxLayout;
		o->addWidget(box4, 1);
		o->addWidget(box5, 1);
		list->addLayout(o);

		auto g = new QHBoxLayout;
		int ef = Setting::getValue("/Danmaku/Effect", 5);
		bold = new QCheckBox(tr("Bold"), widgetPlaying);
		bold->setChecked(ef & 1);
		connect(bold, &QCheckBox::stateChanged, [=](int s)
		{
			Setting::setValue("/Danmaku/Effect", (effect->currentIndex() << 1) | (int)(s == Qt::Checked));
		});
		g->addWidget(bold);

		effect = new QComboBox(widgetPlaying);
		effect->setItemDelegate(new QStyledItemDelegate());
		effect->addItem(tr("Stroke"));
		effect->addItem(tr("Projection"));
		effect->addItem(tr("Shadow"));
		effect->setCurrentIndex(ef >> 1);
		connect<void (QComboBox::*)(int)>(effect, &QComboBox::currentIndexChanged, [=](int i)
		{
			Setting::setValue("/Danmaku/Effect", (i << 1) | (int)(bold->checkState() == Qt::Checked));
		});
		g->addWidget(effect);
		QGroupBox* box6 = new QGroupBox(tr("style"), widgetPlaying);
		box6->setLayout(g);

		auto f = new QHBoxLayout;
		dmfont = new QComboBox(widgetPlaying);
		dmfont->setItemDelegate(new QStyledItemDelegate());
		dmfont->addItems(QFontDatabase().families());
		dmfont->setCurrentText(Setting::getValue("/Danmaku/Font", QFont().family()));
		connect(dmfont, &QComboBox::currentTextChanged, [this](QString _font)
		{
			Setting::setValue("/Danmaku/Font", _font);
		});
		f->addWidget(dmfont);
		QGroupBox* box7 = new QGroupBox(tr("font"), widgetPlaying);
		box7->setLayout(f);

		auto v = new QHBoxLayout;
		v->addWidget(box6, 1);
		v->addWidget(box7, 1);
		list->addLayout(v);

		list->addStretch(10);
		tab->addTab(widgetPlaying, tr("Playing"));
	}
}

void Prefer::setupInterface()
{
	//Interface
	{
		widgetInterface = new QWidget(this);
		auto list = new QVBoxLayout(widgetInterface);

		auto s = new QHBoxLayout;
		size = new QLineEdit(widgetInterface);
		size->setText(Setting::getValue("/Interface/Size", QString("720,405")).trimmed());
		connect(size, &QLineEdit::editingFinished, [=]()
		{
			QRegularExpression r("\\D");
			r.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);
			QString s = size->text().trimmed();
			s.replace(r, ",");
			size->setText(s);
			Setting::setValue("/Interface/Size", s + " ");
		});
		s->addWidget(size);
		QGroupBox* ui = new QGroupBox(tr("initialize size"), widgetInterface);
		ui->setLayout(s);

		auto j = new QHBoxLayout;
		jump = new QLineEdit(widgetPlaying);
		jump->setText(QString::number(Setting::getValue("/Interface/Interval", 10), 'f', 2));
		connect(jump, &QLineEdit::editingFinished, [=]()
		{
			Setting::setValue("/Interface/Interval", jump->text().toDouble());
		});
		j->addWidget(jump);
		QGroupBox* ui1 = new QGroupBox(tr("skip time"), widgetInterface);
		ui1->setLayout(j);

		auto q = new QHBoxLayout;
		q->addWidget(ui);
		q->addWidget(ui1);
		list->addLayout(q);

		auto t = new QGridLayout;
		less = new QCheckBox(tr("frameless"), widgetInterface);
		less->setChecked(Setting::getValue("/Interface/Frameless", false));
		connect(less, &QCheckBox::stateChanged, [](int state)
		{
			Setting::setValue("/Interface/Frameless", state == Qt::Checked);
		});
		t->addWidget(less, 0, 1);

		vers = new QCheckBox(tr("version information"), widgetInterface);
		vers->setChecked(Setting::getValue("/Interface/Version", true));
		connect(vers, &QCheckBox::stateChanged, [](int state)
		{
			Setting::setValue("/Interface/Version", state == Qt::Checked);
		});
		t->addWidget(vers, 0, 0);

		sens = new QCheckBox(tr("sensitive pausing"), widgetInterface);
		sens->setChecked(Setting::getValue("/Interface/Sensitive", false));
		connect(sens, &QCheckBox::stateChanged, [](int state)
		{
			Setting::setValue("/Interface/Sensitive", state == Qt::Checked);
		});
		t->addWidget(sens, 1, 0);

		upda = new QCheckBox(tr("check for update"), widgetInterface);
		upda->setChecked(Setting::getValue("/Interface/Update", true));
		connect(upda, &QCheckBox::stateChanged, [](int state)
		{
			Setting::setValue("/Interface/Update", state == Qt::Checked);
		});
		t->addWidget(upda, 1, 1);

		QGroupBox* ui2 = new QGroupBox(tr("window flag"), widgetInterface);
		ui2->setLayout(t);
		list->addWidget(ui2);

		auto f = new QHBoxLayout;
		font = new QComboBox(widgetInterface);
		font->setItemDelegate(new QStyledItemDelegate());
		font->addItems(QFontDatabase().families());
		font->setCurrentText(Setting::getValue("/Interface/Font/Family", QFont().family()));
		connect(font, &QComboBox::currentTextChanged, [=](QString _font)
		{
			Setting::setValue("/Interface/Font/Family", _font);
		});
		f->addWidget(font);

		QGroupBox* ui3 = new QGroupBox(tr("interface font"), widgetInterface);
		ui3->setLayout(f);

		auto r = new QHBoxLayout;
		reop = new QComboBox(widgetInterface);
		reop->setItemDelegate(new QStyledItemDelegate());
		reop->addItems({ tr("open in new window"), tr("open in current window"), tr("append to playlist") });
		reop->setCurrentIndex(Setting::getValue("/Interface/Single", 1));
		connect<void (QComboBox::*)(int)>(reop, &QComboBox::currentIndexChanged, [=](int i)
		{
			Setting::setValue("/Interface/Single", i);
		});
		r->addWidget(reop);

		QGroupBox* ui4 = new QGroupBox(tr("reopen action"), widgetInterface);
		ui4->setLayout(r);

		auto v = new QHBoxLayout;
		v->addWidget(ui3, 1);
		v->addWidget(ui4, 1);
		list->addLayout(v);

		auto l = new QHBoxLayout;
		loca = new QComboBox(widgetInterface);
		loca->setItemDelegate(new QStyledItemDelegate());
		loca->addItem("English", QString());
		for (const QFileInfo &info : QDir("./locale/").entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
		{
			QLocale l(info.baseName());
			QString name = l.name();
			QString text = l.nativeLanguageName();
			loca->addItem(text, name);
			if (name == Setting::getValue("/Interface/Locale", QVariant()))
			{
				loca->setCurrentIndex(loca->count() - 1);
			}
		}
		connect<void(QComboBox::*)(int)>(loca, &QComboBox::currentIndexChanged, [this](int i)
		{
			Setting::setValue("/Interface/Locale", loca->itemData(i).toString());
		});
		l->addWidget(loca);

		QGroupBox* ui5 = new QGroupBox(tr("locale"));
		ui5->setLayout(l);

		auto m = new QHBoxLayout;
		stay = new QComboBox(widgetInterface);
		stay->setItemDelegate(new QStyledItemDelegate());
		stay->addItems({ tr("never"), tr("playing"), tr("always") });
		stay->setCurrentIndex(Setting::getValue("/Interface/Top", 0));
		connect<void(QComboBox::*)(int)>(stay, &QComboBox::currentIndexChanged, [this](int i)
		{
			Setting::setValue("/Interface/Top", i);
		});
		m->addWidget(stay);
		QGroupBox* ui6 = new QGroupBox(tr("stay on top"));
		ui6->setLayout(m);

		auto n = new QHBoxLayout;
		n->addWidget(ui5);
		n->addWidget(ui6);
		list->addLayout(n);


		list->addStretch(10);
		tab->addTab(widgetInterface, tr("Interface"));
	}
}

void Prefer::setupShortcut()
{
	//Shortcut
	{
		widgetShortcut = new QWidget(this);
		auto w = new QGridLayout(widgetShortcut);
		hotkey = new QTreeWidget(widgetShortcut);
		hotkey->setSelectionMode(QAbstractItemView::NoSelection);
		hotkey->setColumnCount(2);
		hotkey->header()->hide();
		hotkey->header()->setStretchLastSection(false);
		hotkey->header()->setSectionResizeMode(0, QHeaderView::Stretch);
		hotkey->setColumnWidth(1, 1.2*logicalDpiX());
		hotkey->setEditTriggers(QAbstractItemView::NoEditTriggers);
		QAction *iter = new QAction("test");
		iter->setShortcut(QKeySequence("Ctrl+K"));
		{
			QTreeWidgetItem *item = new QTreeWidgetItem;
			item->setData(0, Qt::DisplayRole, iter->text());
			item->setData(1, Qt::DisplayRole, iter->shortcut().toString());
			item->setData(1, Qt::UserRole, iter->objectName());
			item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
			item->setSizeHint(0, QSize(60, 25 * logicalDpiY() / 72));
			hotkey->addTopLevelItem(item);
		}
		w->addWidget(hotkey);
		connect(hotkey, &QTreeWidget::currentItemChanged, [this]()
		{
			hotkey->setCurrentItem(nullptr);
		});
		connect(hotkey, &QTreeWidget::itemClicked, [=](QTreeWidgetItem *item, int column)
		{
			if (column == 1)
			{
				hotkey->editItem(item, 1);
			}
		});

		tab->addTab(widgetShortcut, tr("Shortcut"));
	}
}

void Prefer::saveSettings()
{
	if (reparse != getReparse())
	{
		Danmaku::instance()->parse(0x2);
	}
	Setting::save();
}

QHash<QString, QVariant> Prefer::getRestart()
{
	QStringList path;
	path << "/Performance" <<
		"/Interface/Font" <<
		"/Interface/Frameless" <<
		"/Interface/Single" <<
		"/Interface/Version" <<
		"/Interface/Locale" <<
		"/Plugin";
	QHash<QString, QVariant> data;
	for (QString iter : path)
	{
		data[iter] = Setting::getValue<QVariant>(iter);
	}
	return data;
}

QHash<QString, QVariant> Prefer::getReparse()
{
	QHash<QString, QVariant> data;
	data["/Shield/Limit"] = Setting::getValue("/Shield/Limit", QVariant());
	return data;
}

Prefer::Prefer(QWidget *parent) :
QDialog(parent)
{
	setWindowTitle(tr("Settings"));
	//
	setStyleSheet("QAbstractItemView::item {min-height:16px;}");
	//
	auto outer = new QGridLayout(this);
	tab = new QTabWidget(this);
	outer->addWidget(tab);
	//
	setupPlay();
	setupInterface();
	setupShortcut();
	//
	int h = outer->minimumSize().height();
	int w = 1.15*h;
	setMinimumSize(w, h);
	Utils::setCenter(this);
	//
	restart = getRestart();
	reparse = getReparse();
	//
	connect(tab, &QTabWidget::currentChanged, [this](int index)
	{
		tab->widget(index)->setFocus();
	});
	//
	connect(this, &QDialog::finished, [=]() {saveSettings(); });
}
