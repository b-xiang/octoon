#include "material_window.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <qapplication.h>
#include <fstream>
#include <codecvt>
#include <qtreewidget.h>
#include <qpropertyanimation.h>

namespace rabbit
{
	class DoubleSpinBox final : public QDoubleSpinBox
	{
	public:
		void focusInEvent(QFocusEvent* event) override
		{
			this->grabKeyboard();
			QDoubleSpinBox::focusInEvent(event);
		}

		void focusOutEvent(QFocusEvent* event) override
		{
			this->releaseKeyboard();
			QDoubleSpinBox::focusOutEvent(event);
		}
	};

	Spoiler::Spoiler(const QString& title, const int animationDuration, QWidget* parent) : QWidget(parent), animationDuration(animationDuration) {
		toggleButton.setStyleSheet("QToolButton { border: none; }");
		toggleButton.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		toggleButton.setArrowType(Qt::ArrowType::RightArrow);
		toggleButton.setText(title);
		toggleButton.setCheckable(true);
		toggleButton.setChecked(false);

		headerLine.setFrameShape(QFrame::HLine);
		headerLine.setFrameShadow(QFrame::Sunken);
		headerLine.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

		contentArea.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		// start out collapsed
		contentArea.setMaximumHeight(0);
		contentArea.setMinimumHeight(0);
		// let the entire widget grow and shrink with its content
		toggleAnimation.addAnimation(new QPropertyAnimation(this, "minimumHeight"));
		toggleAnimation.addAnimation(new QPropertyAnimation(this, "maximumHeight"));
		toggleAnimation.addAnimation(new QPropertyAnimation(&contentArea, "maximumHeight"));
		// don't waste space
		mainLayout.setVerticalSpacing(0);
		mainLayout.setContentsMargins(0, 0, 0, 0);
		int row = 0;
		mainLayout.addWidget(&toggleButton, row, 0, 1, 1, Qt::AlignLeft);
		mainLayout.addWidget(&headerLine, row++, 2, 1, 1);
		mainLayout.addWidget(&contentArea, row, 0, 1, 3);
		setLayout(&mainLayout);
		QObject::connect(&toggleButton, &QToolButton::clicked, [this](const bool checked) {
			toggleButton.setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
			toggleAnimation.setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
			toggleAnimation.start();
		});
	}

	void Spoiler::setContentLayout(QLayout& contentLayout) {
		delete contentArea.layout();
		contentArea.setLayout(&contentLayout);
		const auto collapsedHeight = sizeHint().height() - contentArea.maximumHeight();
		auto contentHeight = contentLayout.sizeHint().height();
		for (int i = 0; i < toggleAnimation.animationCount() - 1; ++i) {
			QPropertyAnimation* spoilerAnimation = static_cast<QPropertyAnimation*>(toggleAnimation.animationAt(i));
			spoilerAnimation->setDuration(animationDuration);
			spoilerAnimation->setStartValue(collapsedHeight);
			spoilerAnimation->setEndValue(collapsedHeight + contentHeight);
		}
		QPropertyAnimation* contentAnimation = static_cast<QPropertyAnimation*>(toggleAnimation.animationAt(toggleAnimation.animationCount() - 1));
		contentAnimation->setDuration(animationDuration);
		contentAnimation->setStartValue(0);
		contentAnimation->setEndValue(contentHeight);
	}

	MaterialModifyWindow::MaterialModifyWindow(QWidget* widget)
		: QWidget(widget)
	{
		okButton_ = new QToolButton();
		okButton_->setText(u8"确定");

		auto mainLayout_ = new QVBoxLayout(this);
		mainLayout_->addWidget(this->createSummary(), 0, Qt::AlignTop);
		mainLayout_->addWidget(this->createAlbedo(), 0,Qt::AlignTop);
		mainLayout_->addWidget(this->createNormal(), 0,  Qt::AlignTop);
		mainLayout_->addWidget(this->createSmoothness(), 0,  Qt::AlignTop);
		mainLayout_->addWidget(this->createMetalness(), 0, Qt::AlignTop);
		mainLayout_->addWidget(this->createEmissive(), 0, Qt::AlignTop);
		mainLayout_->addStretch(500);
		mainLayout_->addWidget(okButton_, 0, Qt::AlignBottom | Qt::AlignRight);
		mainLayout_->setContentsMargins(0, 10, 20, 10);

		connect(smoothnessSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(smoothEditEvent(double)));
		connect(smoothnessSlider_, SIGNAL(valueChanged(int)), this, SLOT(smoothSliderEvent(int)));
		connect(metalnessSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(metalEditEvent(double)));
		connect(metalnessSlider_, SIGNAL(valueChanged(int)), this, SLOT(metalSliderEvent(int)));
		connect(albedoColor_, SIGNAL(currentColorChanged(QColor)), this, SLOT(albedoColorChanged(QColor)));
		connect(emissiveColor_, SIGNAL(currentColorChanged(QColor)), this, SLOT(emissiveColorChanged(QColor)));
	}

	MaterialModifyWindow::~MaterialModifyWindow()
	{
	}

	QWidget*
	MaterialModifyWindow::createSummary()
	{
		QPixmap pixmap(":res/icons/material.png");

		imageLabel_ = new QLabel();
		imageLabel_->setFixedSize(QSize(160, 160));
		imageLabel_->setPixmap(pixmap.scaled(160, 160));

		textLabel_ = new QLabel();
		textLabel_->setText(u8"material");

		QVBoxLayout* summaryLayout = new QVBoxLayout;
		summaryLayout->setMargin(0);
		summaryLayout->setSpacing(0);
		summaryLayout->addWidget(imageLabel_, 0, Qt::AlignCenter);
		summaryLayout->addWidget(textLabel_, 0, Qt::AlignCenter);
		summaryLayout->setContentsMargins(0, 0, 50, 0);

		QWidget* summaryWidget = new QWidget;
		summaryWidget->setLayout(summaryLayout);

		return summaryWidget;
	}

	QWidget*
	MaterialModifyWindow::createAlbedo()
	{
		albedoColor_ = new ColorDialog();
		albedoColor_->setMaximumWidth(260);
		albedoColor_->setCurrentColor(QColor(255, 255, 255));

		auto albedoLayout = new QVBoxLayout();
		albedoLayout->addWidget(albedoColor_);

		auto baseColor = new Spoiler(u8"基本颜色");
		baseColor->setFixedWidth(340);
		baseColor->setContentLayout(*albedoLayout);

		return baseColor;
	}

	QWidget*
	MaterialModifyWindow::createNormal()
	{
		QPixmap pixmap(":res/icons/material.png");

		normalLabel_ = new QLabel();
		normalLabel_->setFixedSize(QSize(160, 160));
		normalLabel_->setPixmap(pixmap.scaled(160, 160));

		auto normalLayout = new QVBoxLayout();
		normalLayout->addWidget(normalLabel_);

		auto normal = new Spoiler(u8"法线");
		normal->setFixedWidth(340);
		normal->setContentLayout(*normalLayout);

		return normal;
	}

	QWidget*
	MaterialModifyWindow::createSmoothness()
	{
		smoothnessLabel_ = new QLabel;
		smoothnessLabel_->setText(u8"光滑度");

		smoothnessSlider_ = new QSlider;
		smoothnessSlider_->setObjectName("Value");
		smoothnessSlider_->setOrientation(Qt::Horizontal);
		smoothnessSlider_->setMinimum(0);
		smoothnessSlider_->setMaximum(100);
		smoothnessSlider_->setValue(0);
		smoothnessSlider_->setFixedWidth(260);

		smoothnessSpinBox_ = new DoubleSpinBox;
		smoothnessSpinBox_->setFixedWidth(50);
		smoothnessSpinBox_->setMaximum(1.0f);
		smoothnessSpinBox_->setSingleStep(0.03f);
		smoothnessSpinBox_->setAlignment(Qt::AlignRight);
		smoothnessSpinBox_->setValue(0.0f);

		auto smoothnessHLayout = new QHBoxLayout();
		smoothnessHLayout->addWidget(smoothnessLabel_, 0, Qt::AlignLeft);
		smoothnessHLayout->addWidget(smoothnessSpinBox_, 0, Qt::AlignRight);

		auto smoothnessLayout = new QVBoxLayout();
		smoothnessLayout->addLayout(smoothnessHLayout);
		smoothnessLayout->addWidget(smoothnessSlider_);
		smoothnessLayout->setContentsMargins(30, 5, 50, 0);

		auto smoothness = new Spoiler(u8"光滑度");
		smoothness->setFixedWidth(340);
		smoothness->setContentLayout(*smoothnessLayout);

		return smoothness;
	}

	QWidget*
	MaterialModifyWindow::createMetalness()
	{
		metalnessLabel_ = new QLabel;
		metalnessLabel_->setText(u8"金属程度");

		metalnessSlider_ = new QSlider;
		metalnessSlider_->setObjectName("Value");
		metalnessSlider_->setOrientation(Qt::Horizontal);
		metalnessSlider_->setMinimum(0);
		metalnessSlider_->setMaximum(100);
		metalnessSlider_->setValue(0);
		metalnessSlider_->setFixedWidth(260);

		metalnessSpinBox_ = new DoubleSpinBox;
		metalnessSpinBox_->setFixedWidth(50);
		metalnessSpinBox_->setMaximum(1.0f);
		metalnessSpinBox_->setSingleStep(0.03f);
		metalnessSpinBox_->setAlignment(Qt::AlignRight);
		metalnessSpinBox_->setValue(0.0f);

		auto metalnessHLayout = new QHBoxLayout();
		metalnessHLayout->addWidget(metalnessLabel_, 0, Qt::AlignLeft);
		metalnessHLayout->addWidget(metalnessSpinBox_, 0, Qt::AlignRight);

		auto metalnessLayout = new QVBoxLayout();
		metalnessLayout->addLayout(metalnessHLayout);
		metalnessLayout->addWidget(metalnessSlider_);
		metalnessLayout->setContentsMargins(30, 5, 50, 0);

		auto metalness = new Spoiler(u8"金属程度");
		metalness->setFixedWidth(340);
		metalness->setContentLayout(*metalnessLayout);

		return metalness;
	}

	QWidget*
	MaterialModifyWindow::createEmissive()
	{
		emissiveColor_ = new ColorDialog();
		emissiveColor_->setMaximumWidth(260);
		emissiveColor_->setCurrentColor(QColor(255, 255, 255));

		auto emissiveLayout = new QVBoxLayout();
		emissiveLayout->addWidget(emissiveColor_);

		auto emissive = new Spoiler(u8"自发光");
		emissive->setFixedWidth(340);
		emissive->setContentLayout(*emissiveLayout);

		return emissive;
	}

	void
	MaterialModifyWindow::setMaterial(const std::shared_ptr<octoon::material::Material>& material)
	{
		if (this->material_ != material)
		{
			auto standard = material->downcast_pointer<octoon::material::MeshStandardMaterial>();

			auto colorTexture = standard->getColorTexture();
			if (colorTexture) {
				QPixmap pixmap;
				pixmap.load(QString::fromStdString(colorTexture->getTextureDesc().getName()));
				imageLabel_->setPixmap(pixmap.scaled(130, 130));
			}

			textLabel_->setText(QString::fromStdString(standard->getName()));
			albedoColor_->setCurrentColor(QColor::fromRgbF(standard->getColor().x, standard->getColor().y, standard->getColor().z));
			smoothnessSpinBox_->setValue(standard->getSmoothness());
			metalnessSpinBox_->setValue(standard->getMetalness());
			emissiveColor_->setCurrentColor(QColor::fromRgbF(standard->getEmissive().x, standard->getEmissive().y, standard->getEmissive().z));

			this->material_ = material;
		}
	}

	void
	MaterialModifyWindow::albedoColorChanged(QColor color)
	{
		if (this->material_)
		{
			auto standard = this->material_->downcast_pointer<octoon::material::MeshStandardMaterial>();
			standard->setColor(octoon::math::float3(color.redF(), color.greenF(), color.blueF()));
		}
	}

	void
	MaterialModifyWindow::emissiveColorChanged(QColor color)
	{
		if (this->material_)
		{
			auto standard = this->material_->downcast_pointer<octoon::material::MeshStandardMaterial>();
			standard->setColor(octoon::math::float3(color.redF(), color.greenF(), color.blueF()));
		}
	}

	void
	MaterialModifyWindow::smoothEditEvent(double value)
	{
		smoothnessSlider_->setValue(value * 100.f);

		if (this->material_)
		{
			auto standard = this->material_->downcast_pointer<octoon::material::MeshStandardMaterial>();
			standard->setSmoothness(value);
		}
	}

	void
	MaterialModifyWindow::smoothSliderEvent(int value)
	{
		smoothnessSpinBox_->setValue(value / 100.0f);
	}

	void
	MaterialModifyWindow::metalEditEvent(double value)
	{
		metalnessSlider_->setValue(value * 100.f);

		if (this->material_)
		{
			auto standard = this->material_->downcast_pointer<octoon::material::MeshStandardMaterial>();
			standard->setMetalness(value);
		}
	}

	void
	MaterialModifyWindow::metalSliderEvent(int value)
	{
		metalnessSpinBox_->setValue(value / 100.0f);
	}

	void 
	MaterialModifyWindow::closeEvent()
	{
		this->hide();
	}

	MaterialWindow::MaterialWindow(QWidget* parent, const octoon::GameObjectPtr& behaviour) noexcept
		: behaviour_(behaviour)
	{
		this->hide();
		this->setObjectName("materialWindow");
		this->setWindowTitle(u8"材质");
		this->setFixedWidth(340);
		this->setAcceptDrops(true);

		title_ = std::make_unique<QLabel>();
		title_->setText(u8"材质");

		closeButton_ = std::make_unique<QToolButton>();
		closeButton_->setObjectName("close");
		closeButton_->setToolTip(u8"关闭");

		listWidget_ = std::make_unique<QListWidget>();
		listWidget_->setIconSize(QSize(210, 210));
		listWidget_->setResizeMode(QListView::Adjust);
		listWidget_->setViewMode(QListView::IconMode);
		listWidget_->setMovement(QListView::Static);
		listWidget_->setSpacing(10);
		listWidget_->setMinimumHeight(this->height());
		listWidget_->setMinimumWidth(this->width());
		listWidget_->setStyleSheet("background:transparent;");
		listWidget_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	
		titleLayout_ = std::make_unique<QHBoxLayout>();
		titleLayout_->addWidget(title_.get(), 0, Qt::AlignLeft);
		titleLayout_->addWidget(closeButton_.get(), 0, Qt::AlignRight);

		modifyWidget_ = std::make_unique<MaterialModifyWindow>(this);
		modifyWidget_->setFixedWidth(340);

		modifyMaterialArea_ = new QScrollArea();
		modifyMaterialArea_->setFixedHeight(700);
		modifyMaterialArea_->setWidget(modifyWidget_.get());
		modifyMaterialArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		modifyMaterialArea_->setWidgetResizable(true);
		modifyMaterialArea_->hide();

		mainLayout_ = std::make_unique<QVBoxLayout>(this);
		mainLayout_->addLayout(titleLayout_.get());
		mainLayout_->addWidget(listWidget_.get(), 0, Qt::AlignTop | Qt::AlignCenter);
		mainLayout_->addWidget(modifyMaterialArea_, 0, Qt::AlignTop | Qt::AlignCenter);
		mainLayout_->addStretch(500);
		mainLayout_->setContentsMargins(10, 10, 10, 10);

		connect(closeButton_.get(), SIGNAL(clicked()), this, SLOT(closeEvent()));
		connect(modifyWidget_->okButton_, SIGNAL(clicked()), this, SLOT(okEvent()));
		connect(listWidget_.get(), SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked(QListWidgetItem*)));
		connect(listWidget_.get(), SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));
	}

	MaterialWindow::~MaterialWindow() noexcept
	{
		title_.reset();
		closeButton_.reset();
		titleLayout_.reset();
		mainLayout_.reset();
	}

	void
	MaterialWindow::showEvent(QShowEvent* event) noexcept
	{
		this->repaint();
		this->updateList();
		QMargins margins = mainLayout_->contentsMargins();
		listWidget_->setMinimumHeight(this->height() - title_->height() * 2 - margins.top() - margins.bottom());
	}

	void
	MaterialWindow::closeEvent()
	{
		this->close();
		parentWidget()->setFixedWidth(parentWidget()->width() - this->width());
	}

	void
	MaterialWindow::okEvent()
	{
		modifyMaterialArea_->hide();
		listWidget_->show();
	}

	void
	MaterialWindow::itemClicked(QListWidgetItem* item)
	{
		if (behaviour_)
		{
			auto behaviour = behaviour_->getComponent<rabbit::RabbitBehaviour>();
			if (behaviour->isOpen())
			{
				auto selectedItem = behaviour->getComponent<DragComponent>()->getSelectedItem();
				if (selectedItem)
				{
					auto hit = selectedItem.value();
					auto materialComponent = behaviour->getComponent<MaterialComponent>();
					auto material = materialComponent->getMaterial(item->text().toStdString());

					auto meshRenderer = hit.object->getComponent<octoon::MeshRendererComponent>();
					if (meshRenderer)
						meshRenderer->setMaterial(material, hit.mesh);
				}
			}
		}
	}

	void
	MaterialWindow::itemDoubleClicked(QListWidgetItem* item)
	{
		if (behaviour_)
		{
			auto behaviour = behaviour_->getComponent<rabbit::RabbitBehaviour>();
			if (behaviour->isOpen())
			{
				auto materialComponent = behaviour->getComponent<MaterialComponent>();
				auto material = materialComponent->getMaterial(item->text().toStdString());

				listWidget_->hide();
				modifyWidget_->setMaterial(material);
				modifyMaterialArea_->show();
			}
		}
	}

	void
	MaterialWindow::dragEnterEvent(QDragEnterEvent* event) noexcept
	{
		event->acceptProposedAction();
		event->accept();
	}

	void
	MaterialWindow::dropEvent(QDropEvent* event) noexcept
	{
		auto urls = event->mimeData()->urls();
		if (!urls.isEmpty())
		{
			std::vector<std::wstring> paths;
			for (auto& it : urls)
			{
				auto path = it.toString().toStdWString();
				if (path.find(L"file:///") == 0)
					path = path.substr(8);
			}

			event->accept();
		}
	}

	std::string
	MaterialWindow::currentItem() const noexcept
	{
		return this->listWidget_->currentItem()->text().toStdString();
	}

	void
	MaterialWindow::updateList()
	{
		auto behaviour = behaviour_->getComponent<rabbit::RabbitBehaviour>();
		if (behaviour)
		{
			auto materialComponent = behaviour->getComponent<MaterialComponent>();
			auto& materials = materialComponent->getMaterialList();

			listWidget_->clear();

			std::map<QString, std::shared_ptr<QPixmap>> imageTable;

			for (auto& it : materials)
			{
				std::string path;
				std::string normalName;
				std::string textureName;

				auto mat = it.second;

				if (mat.find("preview") != mat.end())
					textureName = mat["preview"].get<nlohmann::json::string_t>();

				octoon::math::float3 base(mat["color"][0], mat["color"][1], mat["color"][2]);
				octoon::math::float3 specular = octoon::math::float3::One;
				octoon::math::float3 ambient = octoon::math::float3::Zero;
				octoon::math::float4 edgeColor = octoon::math::float4::Zero;

				QListWidgetItem* item = new QListWidgetItem;
				item->setText(QString::fromStdString(mat["uuid"]));
				item->setSizeHint(QSize(130, 160));

				QLabel* imageLabel = new QLabel;

				if (textureName.empty())
				{
					QPixmap pixmap(":res/icons/material.png");
					imageLabel->setPixmap(pixmap);
				}
				else
				{
					auto texpath = QString::fromStdString(path + textureName);
					if (!imageTable[texpath])
						imageTable[texpath] = std::make_shared<QPixmap>(texpath);

					imageLabel->setPixmap(*imageTable[texpath]);
				}

				QLabel* txtLabel = new QLabel(QString::fromStdString(mat["name"]));
				txtLabel->setFixedHeight(30);

				QVBoxLayout* widgetLayout = new QVBoxLayout;
				widgetLayout->setMargin(0);
				widgetLayout->setSpacing(0);
				widgetLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
				widgetLayout->addWidget(txtLabel, 0, Qt::AlignCenter);

				QWidget* widget = new QWidget;
				widget->setLayout(widgetLayout);

				listWidget_->addItem(item);
				listWidget_->setItemWidget(item, widget);
			}
		}
	}
}