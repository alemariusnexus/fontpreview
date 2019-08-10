#include "MainWindow.h"
#include <QFontDatabase>
#include <QFont>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QTimer>
#include <QDir>
#include <cstdio>



MainWindow::MainWindow()
		: inhibitFontListRebuildOnChange(false), unsavedPreviewTextChanges(false)
{
	ui.setupUi(this);

	QSettings settings;

	connect(ui.previewTextTemplateList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(previewTextTemplateListItemChanged(QListWidgetItem*, QListWidgetItem*)));
	connect(ui.previewTextTemplateList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(previewTextTemplateListItemActivated(QListWidgetItem*)));
	connect(ui.previewTextTemplateList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(previewTextTemplateListItemEdited(QListWidgetItem*)));
	connect(ui.previewTextTemplateNewButton, SIGNAL(clicked(bool)), this, SLOT(previewTextTemplateNewButtonClicked()));
	connect(ui.previewTextTemplateRemoveButton, SIGNAL(clicked(bool)), this, SLOT(previewTextTemplateRemoveButtonClicked()));
	connect(ui.previewTextTemplateSaveButton, SIGNAL(clicked(bool)), this, SLOT(previewTextTemplateSaveButtonClicked()));

	connect(ui.textEdit, SIGNAL(textChanged()), this, SLOT(sampleTextChanged()));

	connect(ui.fontShowSystemBox, SIGNAL(stateChanged(int)), this, SLOT(fontShowSystemBoxChanged()));
	connect(ui.fontOpenAction, SIGNAL(triggered(bool)), this, SLOT(fontOpenButtonClicked()));
	connect(ui.fontFamilyList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(fontFamilyListItemChanged(QListWidgetItem*, QListWidgetItem*)));
	connect(ui.fontStyleList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(fontStyleListItemChanged(QListWidgetItem*, QListWidgetItem*)));
	connect(ui.fontSizeList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(fontSizeListItemChanged(QListWidgetItem*, QListWidgetItem*)));

	connect(ui.quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

	connect(ui.mainSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(mainSplitterMoved()));


	ui.previewTextTemplateNewButton->setIcon(QIcon::fromTheme("list-add", QIcon(":/fontpreview/resources/list-add.png")));
	ui.previewTextTemplateRemoveButton->setIcon(QIcon::fromTheme("list-remove", QIcon(":/fontpreview/resources/list-remove.png")));
	ui.previewTextTemplateSaveButton->setIcon(QIcon::fromTheme("document-save", QIcon(":/fontpreview/resources/document-save.png")));

	ui.previewTextTemplateNewButton->setText(QString());
	ui.previewTextTemplateRemoveButton->setText(QString());
	ui.previewTextTemplateSaveButton->setText(QString());


	QString text = settings.value("state/lastSampleText", "The quick brown fox jumps over the lazy dog").toString();
	ui.textEdit->setPlainText(text);

	ui.fontShowSystemBox->setChecked(settings.value("state/fontShowSystem", true).toBool());

	updateAvailableFonts();


	QMap<QString, QVariant> templates = settings.value("general/previewTextTemplates").toMap();

	for (auto tpl : templates.toStdMap()) {
		QString tplName = tpl.first;
		QString tplText = tpl.second.toString();

		QListWidgetItem* item = new QListWidgetItem(tplName);
		item->setData(Qt::UserRole, tplText);
		item->setFlags(item->flags() | Qt::ItemIsEditable);

		ui.previewTextTemplateList->addItem(item);
	}

	ui.previewTextTemplateRemoveButton->setEnabled(ui.previewTextTemplateList->currentItem() != nullptr);

	QTimer::singleShot(0, this, SLOT(loadGeometryFromSettings()));

	setWindowIcon(QIcon(":/fontpreview/resources/app-icon.png"));
	updateWindowTitle();
}


void MainWindow::loadGeometryFromSettings()
{
	QSettings settings;

	if (settings.contains("state/mainWindowGeometry")) {
		restoreGeometry(settings.value("state/mainWindowGeometry").toByteArray());
	}

	if (settings.contains("state/mainSplitterState")) {
		ui.mainSplitter->restoreState(settings.value("state/mainSplitterState").toByteArray());
	}
}


void MainWindow::closeEvent(QCloseEvent* evt)
{
	QSettings settings;

	settings.setValue("state/mainWindowGeometry", saveGeometry());

	QMainWindow::closeEvent(evt);
}


void MainWindow::openFontFile(QString fontFile)
{
	int fontId = QFontDatabase::addApplicationFont(fontFile);

	if (fontId == -1) {
		QMessageBox::critical(this, "Failed To Open Font File", QString("Failed to open font file:\n\n%s").arg(fontFile));
		return;
	}

	QStringList families = QFontDatabase::applicationFontFamilies(fontId);

	if (families.empty()) {
		QMessageBox::warning(this, "Empty Font File", "Font file contains no font families!");
		return;
	}

	for (QString fam : families) {
		customFontFamilies.append(fam);
	}

	QString family = families[0];

	updateAvailableFonts(family);
}


void MainWindow::useFontFamily(const QString& family)
{
	for (int i = 0 ; i < ui.fontFamilyList->count() ; i++) {
		QString ifam = ui.fontFamilyList->item(i)->data(Qt::UserRole).toString();

		if (ifam == family) {
			ui.fontFamilyList->setCurrentRow(i);
			break;
		}
	}
}


void MainWindow::sampleTextChanged()
{
	QSettings settings;

	QString text = ui.textEdit->toPlainText();
	ui.displayEdit->setPlainText(text);

	settings.setValue("state/lastSampleText", text);

	unsavedPreviewTextChanges = true;
	updatePreviewTextTemplateButtons();
}


void MainWindow::previewTextTemplateListItemChanged(QListWidgetItem* cur, QListWidgetItem* prev)
{
	updatePreviewTextTemplateButtons();

	if (cur) {
		QString sampleText = cur->data(Qt::UserRole).toString();
		ui.textEdit->setPlainText(sampleText);
	}
}


void MainWindow::previewTextTemplateListItemActivated(QListWidgetItem* cur)
{
	// Unused, double click edits items instead
}


void MainWindow::previewTextTemplateListItemEdited(QListWidgetItem* item)
{
	savePreviewTextTemplates();
}


void MainWindow::previewTextTemplateNewButtonClicked()
{
	QString tplName;

	for (int i = 1 ; ; i++) {
		tplName = QString("New Template %1").arg(i);

		if (ui.previewTextTemplateList->findItems(tplName, Qt::MatchExactly).empty()) {
			break;
		}
	}

	QListWidgetItem* item = new QListWidgetItem(tplName);
	item->setData(Qt::UserRole, ui.textEdit->toPlainText());
	item->setFlags(item->flags() | Qt::ItemIsEditable);

	ui.previewTextTemplateList->addItem(item);

	ui.previewTextTemplateList->setCurrentItem(item);
	ui.previewTextTemplateList->scrollToItem(item);

	savePreviewTextTemplates();
}


void MainWindow::previewTextTemplateRemoveButtonClicked()
{
	QListWidgetItem* item = ui.previewTextTemplateList->currentItem();

	if (!item) {
		return;
	}

	ui.previewTextTemplateList->takeItem(ui.previewTextTemplateList->row(item));

	savePreviewTextTemplates();
}


void MainWindow::previewTextTemplateSaveButtonClicked()
{
	QListWidgetItem* item = ui.previewTextTemplateList->currentItem();

	if (!item) {
		return;
	}

	item->setData(Qt::UserRole, ui.textEdit->toPlainText());

	savePreviewTextTemplates();

	unsavedPreviewTextChanges = false;

	updatePreviewTextTemplateButtons();
}


void MainWindow::fontOpenButtonClicked()
{
	QSettings settings;

	QString cdir = settings.value("state/fontOpenDialogLastDir").toString();
	QString fontFile = QFileDialog::getOpenFileName(this, "Open Font File", cdir, "Font Files (*.ttf *.ttc *.otf *.otc)");

	if (fontFile.isNull()) {
		return;
	}

	settings.setValue("state/fontOpenDialogLastDir", QFileInfo(fontFile).absoluteDir().absolutePath());

	openFontFile(fontFile);
}


void MainWindow::updateAvailableFonts(const QString& selectedFamily)
{
	QSettings settings;

	QString selFamily = selectedFamily;

	if (selFamily.isNull()) {
		selFamily = settings.value("state/lastFontFamily").toString();
	}

	inhibitFontListRebuildOnChange = true;

	ui.fontFamilyList->clear();

	QListWidgetItem* curItem = nullptr;

	bool showSystemFonts = ui.fontShowSystemBox->isChecked();

	for (QString fam : fdb.families()) {
		bool custom = false;

		if (customFontFamilies.contains(fam, Qt::CaseSensitive)) {
			custom = true;
		}

		if (showSystemFonts  ||  custom) {
			QFont font(fam);
			QListWidgetItem* item = new QListWidgetItem(fam);
			item->setData(Qt::UserRole, fam);
			//item->setFont(font);
			ui.fontFamilyList->addItem(item);

			if (fam == selFamily) {
				curItem = item;
			}
		}
	}

	if (curItem) {
		ui.fontFamilyList->setCurrentItem(curItem);
		ui.fontFamilyList->scrollToItem(curItem);
	}

	updateFontStyleList();

	inhibitFontListRebuildOnChange = false;
}


void MainWindow::updateUsedFont()
{
	QString family = ui.fontFamilyList->currentItem()->data(Qt::UserRole).toString();
	QString style = ui.fontStyleList->currentItem()->data(Qt::UserRole).toString();
	int size = ui.fontSizeList->currentItem()->data(Qt::UserRole).toInt();

	QFont font(family);
	font.setStyleName(style);
	font.setPointSize(size);

	ui.displayEdit->setFont(font);
}


void MainWindow::updatePreviewTextTemplateButtons()
{
	QListWidgetItem* cur = ui.previewTextTemplateList->currentItem();

	ui.previewTextTemplateRemoveButton->setEnabled(cur != nullptr);
	ui.previewTextTemplateSaveButton->setEnabled(cur != nullptr  &&  unsavedPreviewTextChanges);
}


void MainWindow::updateFontStyleList()
{
	QSettings settings;

	inhibitFontListRebuildOnChange = true;

	QString prevStyle;

	if (ui.fontStyleList->currentItem()) {
		prevStyle = ui.fontStyleList->currentItem()->data(Qt::UserRole).toString();
	} else {
		prevStyle = settings.value("state/lastFontStyle").toString();
	}

	ui.fontStyleList->clear();

	if (ui.fontFamilyList->currentItem()) {
		QString family = ui.fontFamilyList->currentItem()->data(Qt::UserRole).toString();

		QListWidgetItem* curItem = nullptr;

		for (QString style : fdb.styles(family)) {
			QListWidgetItem* item = new QListWidgetItem(style);
			item->setData(Qt::UserRole, style);
			ui.fontStyleList->addItem(item);

			if (!prevStyle.isNull()  &&  style.toLower() == prevStyle.toLower()) {
				curItem = item;
			}
		}

		if (!curItem) {
			curItem = ui.fontStyleList->item(0);
		}

		ui.fontStyleList->setCurrentItem(curItem);
		ui.fontStyleList->scrollToItem(curItem);
	}

	updateFontSizeList();

	inhibitFontListRebuildOnChange = false;
}


void MainWindow::updateFontSizeList()
{
	QSettings settings;

	inhibitFontListRebuildOnChange = true;

	int prevSize = -1;

	if (ui.fontSizeList->currentItem()) {
		prevSize = ui.fontSizeList->currentItem()->data(Qt::UserRole).toInt();
	} else {
		prevSize = settings.value("state/lastFontSize", 12).toInt();
	}

	ui.fontSizeList->clear();

	if (ui.fontStyleList->currentItem()) {
		QString family = ui.fontFamilyList->currentItem()->data(Qt::UserRole).toString();
		QString style = ui.fontStyleList->currentItem()->data(Qt::UserRole).toString();

		int bestSizeDist = INT_MAX;
		QListWidgetItem* curItem = nullptr;

		for (int size : fdb.pointSizes(family, style)) {
			QListWidgetItem* item = new QListWidgetItem(QString("%1").arg(size));
			item->setData(Qt::UserRole, size);
			ui.fontSizeList->addItem(item);

			int dist = abs(size - prevSize);

			if (dist < bestSizeDist) {
				bestSizeDist = dist;
				curItem = item;
			}
		}

		if (curItem) {
			ui.fontSizeList->setCurrentItem(curItem);
			ui.fontSizeList->scrollToItem(curItem);
		}
	}

	inhibitFontListRebuildOnChange = false;
}


void MainWindow::savePreviewTextTemplates()
{
	QSettings settings;

	QMap<QString, QVariant> templates;

	for (int i = 0 ; i < ui.previewTextTemplateList->count() ; i++) {
		QListWidgetItem* item = ui.previewTextTemplateList->item(i);
		QString tplName = item->text();
		QString tplText = item->data(Qt::UserRole).toString();

		templates.insert(tplName, tplText);
	}

	settings.setValue("general/previewTextTemplates", templates);
}


void MainWindow::fontShowSystemBoxChanged()
{
	QSettings settings;

	updateAvailableFonts();

	settings.setValue("state/fontShowSystem", ui.fontShowSystemBox->isChecked());
}


void MainWindow::fontFamilyListItemChanged(QListWidgetItem* cur, QListWidgetItem* prev)
{
	QSettings settings;

	if (!inhibitFontListRebuildOnChange) {
		updateFontStyleList();
	}

	if (cur) {
		settings.setValue("state/lastFontFamily", cur->data(Qt::UserRole).toString());
	}

	updateWindowTitle();
}


void MainWindow::fontStyleListItemChanged(QListWidgetItem* cur, QListWidgetItem* prev)
{
	QSettings settings;

	if (!inhibitFontListRebuildOnChange) {
		updateFontSizeList();
	}

	if (cur) {
		settings.setValue("state/lastFontStyle", cur->data(Qt::UserRole).toString());
	}
}


void MainWindow::fontSizeListItemChanged(QListWidgetItem* cur, QListWidgetItem* prev)
{
	QSettings settings;

	if (cur) {
		QString family = ui.fontFamilyList->currentItem()->data(Qt::UserRole).toString();
		QString style = ui.fontStyleList->currentItem()->data(Qt::UserRole).toString();
		int size = cur->data(Qt::UserRole).toInt();

		updateUsedFont();

		settings.setValue("state/lastFontSize", size);
	}
}


void MainWindow::mainSplitterMoved()
{
	QSettings settings;

	settings.setValue("state/mainSplitterState", ui.mainSplitter->saveState());
}


void MainWindow::updateWindowTitle()
{
	QString family;

	if (ui.fontFamilyList->currentItem()) {
		family = ui.fontFamilyList->currentItem()->data(Qt::UserRole).toString();
	}

	QString title;

	if (family.isNull()) {
		title = "Font Preview";
	} else {
		title = QString("%1 - Font Preview").arg(family);
	}

	setWindowTitle(title);
}


