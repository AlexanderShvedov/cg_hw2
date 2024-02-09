#include "Window.h"

#include <QMouseEvent>
#include <QLabel>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QVBoxLayout>
#include <QScreen>

#include <array>
#include <cmath>
#include <iostream>
#include "App/thirdparty/tinygltf/tiny_gltf.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace {
static QOpenGLFunctions_3_3_Core funcs;
static GLuint texid;
}

Window::Window() noexcept
{
	setFocusPolicy(Qt::StrongFocus);

	timer_.start();

	setMouseTracking(true);
}

Window::~Window()
{
	{
		// Free resources with context bounded.
		const auto guard = bindContext();
		texture_.reset();
		program_.reset();
	}
}

bool loadModel(tinygltf::Model &model, const char *filename) {
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
	if (!warn.empty()) {
		std::cout << "WARN: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cout << "ERR: " << err << std::endl;
	}

	if (!res)
		std::cout << "Failed to load glTF: " << filename << std::endl;
	else
		std::cout << "Loaded glTF: " << filename << std::endl;

	return res;
}


void bindMesh(std::map<int, GLuint>& vbos,
			  tinygltf::Model &model, tinygltf::Mesh &mesh) {
	for (size_t i = 0; i < model.bufferViews.size(); ++i) {
		const tinygltf::BufferView &bufferView = model.bufferViews[i];
		if (bufferView.target == 0) {  // TODO impl drawarrays
			std::cout << "WARN: bufferView.target is zero" << std::endl;
			continue;  // Unsupported bufferView.
		}

		const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

		GLuint vbo;
		funcs.glGenBuffers(1, &vbo);
		vbos[i] = vbo;
		funcs.glBindBuffer(bufferView.target, vbo);

		funcs.glBufferData(bufferView.target, bufferView.byteLength,
						   &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
	}

	for (size_t i = 0; i < mesh.primitives.size(); ++i) {
		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		for (auto &attrib : primitive.attributes) {
			tinygltf::Accessor accessor = model.accessors[attrib.second];
			int byteStride =
				accessor.ByteStride(model.bufferViews[accessor.bufferView]);
			funcs.glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

			int size = 1;
			if (accessor.type != TINYGLTF_TYPE_SCALAR) {
				size = accessor.type;
			}

			int vaa = -1;
			if (attrib.first.compare("POSITION") == 0) vaa = 0;
			if (attrib.first.compare("NORMAL") == 0) vaa = 1;
			if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
			if (vaa > -1) {
				funcs.glEnableVertexAttribArray(vaa);
				funcs.glVertexAttribPointer(vaa, size, accessor.componentType,
											accessor.normalized ? GL_TRUE : GL_FALSE,
											byteStride, BUFFER_OFFSET(accessor.byteOffset));
			} else
				std::cout << "vaa missing: " << attrib.first << std::endl;
		}

		if (model.textures.size() > 0) {
			// fixme: Use material's baseColor
			tinygltf::Texture &tex = model.textures[0];

			if (tex.source > -1) {
				funcs.glGenTextures(1, &texid);

				tinygltf::Image &image = model.images[tex.source];

				funcs.glBindTexture(GL_TEXTURE_2D, texid);
				funcs.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				funcs.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				funcs.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				funcs.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				funcs.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

				GLenum format = GL_RGBA;

				if (image.component == 1) {
					format = GL_RED;
				} else if (image.component == 2) {
					format = GL_RG;
				} else if (image.component == 3) {
					format = GL_RGB;
				} else {
					// ???
				}

				GLenum type = GL_UNSIGNED_BYTE;
				if (image.bits == 8) {
					// ok
				} else if (image.bits == 16) {
					type = GL_UNSIGNED_SHORT;
				} else {
					std::cout << "??? image.bits : " << image.bits << std::endl;
					// ???
				}

				funcs.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0,
								   format, type, &image.image.at(0));
			}
		}
	}
}

// bind models
void bindModelNodes(std::map<int, GLuint>& vbos, tinygltf::Model &model,
					tinygltf::Node &node) {
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		bindMesh(vbos, model, model.meshes[node.mesh]);
	}

	for (size_t i = 0; i < node.children.size(); i++) {
		assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
		bindModelNodes(vbos, model, model.nodes[node.children[i]]);
	}
}

std::pair<GLuint, std::map<int, GLuint>> bindModel(tinygltf::Model &model) {
	std::map<int, GLuint> vbos;
	GLuint vao;
	funcs.glGenVertexArrays(1, &vao);
	funcs.glBindVertexArray(vao);

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
		bindModelNodes(vbos, model, model.nodes[scene.nodes[i]]);
	}

	funcs.glBindVertexArray(0);
	// cleanup vbos but do not delete index buffers yet
	for (auto it = vbos.cbegin(); it != vbos.cend();) {
		tinygltf::BufferView bufferView = model.bufferViews[it->first];
		if (bufferView.target != GL_ELEMENT_ARRAY_BUFFER) {
			funcs.glDeleteBuffers(1, &vbos[it->first]);
			vbos.erase(it++);
		}
		else {
			++it;
		}
	}

	return {vao, vbos};
}

void drawMesh(const std::map<int, GLuint>& vbos,
			  tinygltf::Model &model, tinygltf::Mesh &mesh) {
	for (size_t i = 0; i < mesh.primitives.size(); ++i) {
		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		funcs.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

		funcs.glBindTexture(GL_TEXTURE_2D, texid);

		funcs.glDrawElements(primitive.mode, indexAccessor.count,
							 indexAccessor.componentType,
							 BUFFER_OFFSET(indexAccessor.byteOffset));
	}
}

// recursively draw node and children nodes of model
void drawModelNodes(const std::pair<GLuint, std::map<int, GLuint>>& vaoAndEbos,
					tinygltf::Model &model, tinygltf::Node &node) {
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		drawMesh(vaoAndEbos.second, model, model.meshes[node.mesh]);
	}
	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(vaoAndEbos, model, model.nodes[node.children[i]]);
	}
}
void drawModel(const std::pair<GLuint, std::map<int, GLuint>>& vaoAndEbos,
			   tinygltf::Model &model) {
	funcs.glBindVertexArray(vaoAndEbos.first);

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawModelNodes(vaoAndEbos, model, model.nodes[scene.nodes[i]]);
	}

	funcs.glBindVertexArray(0);
}

void Window::onInit()
{
	funcs.initializeOpenGLFunctions();

	// Configure shaders
	program_ = std::make_unique<QOpenGLShaderProgram>(this);
	program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/diffuse.vs");
	program_->addShaderFromSourceFile(QOpenGLShader::Fragment,
									  ":/Shaders/diffuse.fs");
	program_->link();

	// bind model
	//std::string filename = "/Users/aleksandrsvedov/CLionProjects/cg_hw2/src/App/Models/cassette_tape/scene.gltf";
//	std::string filename = "/Users/aleksandrsvedov/CLionProjects/cg_hw2/src/App/Models/low_poly_apple_game_ready/scene.gltf";
//	std::string filename = "/Users/aleksandrsvedov/CLionProjects/cg_hw2/src/App/Models/toon_cat_free/scene.gltf";
	std::string filename = "/Users/aleksandrsvedov/CLionProjects/cg_hw2/src/App/Models/rubik_cube/scene.gltf";
	if (!loadModel(model_, filename.c_str())) return;

	vaoAndEbos_ = bindModel(model_);

	// Bind attributes
	program_->bind();

	sunUniform_ = program_->uniformLocation("sun_light_coef");
	spotUniform_ = program_->uniformLocation("spotlight_coef");
	ambientUniform_ = program_->uniformLocation("ambient_light_coef");
	mUniform_ = program_->uniformLocation("m");
	vUniform_ = program_->uniformLocation("v");
	pUniform_ = program_->uniformLocation("p");
	sunPositionUniform_ = program_->uniformLocation("sun_position");
	sunColorUniform_ = program_->uniformLocation("sun_color");
	spotlightPositionUniform_ = program_->uniformLocation("spotlight_position");
	spotlightColorUniform_ = program_->uniformLocation("spotlight_color");
	spotlightDirectionUniform_ = program_->uniformLocation("spotlight_direction");
	spotlightFirstCosUniform_ = program_->uniformLocation("spotlight_first_cos");
	spotlightSecondCosUniform_ = program_->uniformLocation("spotlight_second_cos");
	morphingProgressUniform_ = program_->uniformLocation("morhping_progress");

	// Release all
	program_->release();

	// Еnable depth test and face culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Clear all FBO buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create camera
	camera_ = Camera(800, 800, {9.5, 0.0, 1.0});
}

void Window::onRender()
{
	const auto guard = captureMetrics();

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind VAO and shader program
	program_->bind();
	const auto fov = 60.0f;
	const auto zNear = 0.1f;
	const auto zFar = 100.0f;
	auto [m, v, p, direction] = camera_.update(fov, zNear, zFar, totalFrameCount_);
	program_->setUniformValue(sunUniform_, sun);
	program_->setUniformValue(ambientUniform_, ambient);
	program_->setUniformValue(spotUniform_, spot);
	program_->setUniformValue(mUniform_, m);
	program_->setUniformValue(vUniform_, v);
	program_->setUniformValue(pUniform_, p);
	program_->setUniformValue(sunPositionUniform_, lightPos);
	program_->setUniformValue(spotlightPositionUniform_, camera_.position);
	program_->setUniformValue(sunColorUniform_, sunColor_);
	program_->setUniformValue(spotlightColorUniform_, spotlightColor_);
	program_->setUniformValue(spotlightDirectionUniform_, direction);
	program_->setUniformValue(spotlightFirstCosUniform_, GLfloat(std::cos((spotlightFirstAngle_ / 10) * 100/ 180.0f)));
	program_->setUniformValue(spotlightSecondCosUniform_, GLfloat(std::cos((spotlightSecondAngle_ / 10) * 100 / 180.0f)));
	program_->setUniformValue(morphingProgressUniform_, morphingProgress_);

	// Draw
	drawModel(vaoAndEbos_, model_);

	program_->release();

	++frameCount_;
	++totalFrameCount_;

	// Request redraw if animated
	if (animated_)
	{
		update();
	}
}

void Window::onResize(const size_t width, const size_t height)
{
	// Configure viewport
	glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));

	// Update camera
	camera_.resize(width, height);
}

Window::PerfomanceMetricsGuard::PerfomanceMetricsGuard(std::function<void()> callback)
	: callback_{ std::move(callback) }
{
}


void Window::mouseMoveEvent(QMouseEvent* e)
{
	camera_.input(e);
}

Window::PerfomanceMetricsGuard::~PerfomanceMetricsGuard()
{
	if (callback_)
	{
		callback_();
	}
}

auto Window::captureMetrics() -> PerfomanceMetricsGuard
{
	return PerfomanceMetricsGuard{
		[&] {
			if (timer_.elapsed() >= 1000)
			{
				const auto elapsedSeconds = static_cast<float>(timer_.restart()) / 1000.0f;
				uint fps = static_cast<size_t>(std::round(frameCount_ / elapsedSeconds));
				frameCount_ = 0;
				emit updateFPS(fps);
			}
		}
	};
}

void Window::setLightX(float new_x)
{
	lightPos.setX(new_x / 100.0f);
}


void Window::setLightZ(float new_z)
{
	lightPos.setZ(new_z / 100.0f);
}

void Window::setMorphingProgress(float newProgress)
{
	morphingProgress_ = newProgress / 100.0f;
}
void Window::wheelEvent(QWheelEvent * event)
{
	camera_.wheelEvent(event);
}
void Window::mousePressEvent(QMouseEvent * event)
{
	camera_.mousePressEvent(event);
}

void Window::setSpot(float spot)
{
	this->spot = spot;
}

void Window::setAmbient(float ambient)
{
	this->ambient = ambient;
}

void Window::setSun(float sun)
{
	this->sun = sun;
}
