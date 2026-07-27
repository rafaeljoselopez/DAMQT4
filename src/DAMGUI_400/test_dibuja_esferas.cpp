struct Sphere {
    QVector3D center;
    float radius;

    // Puedes añadir aquí color, ID, etc.

    // Método de dibujo para esta esfera
    void draw(QOpenGLFunctions* glFuncs) const {
        // Suponemos que tienes una función de dibujo que acepta posición y radio.
        // Reemplázala por tu método real de renderizado de esferas.
        dibujaEsfera(center, radius, glFuncs);
    }
};


#include <QOpenGLFunctions>
#include <vector>

void computeSphereVisibility(const std::vector<Sphere>& spheres,
                             std::vector<bool>& visibilityMask,
                             QOpenGLFunctions* glFuncs = QOpenGLContext::currentContext()->functions())
{
    if (!glFuncs || spheres.empty()) return;

    const int n = spheres.size();
    visibilityMask.resize(n, false);

    // Paso 1: dibuja toda la escena al z-buffer
    glFuncs->glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glFuncs->glDepthMask(GL_TRUE);
    glFuncs->glEnable(GL_DEPTH_TEST);
    glFuncs->glClear(GL_DEPTH_BUFFER_BIT);

    for (const auto& sphere : spheres) {
        sphere.draw(glFuncs);
    }

    // Paso 2: queries individuales
    std::vector<GLuint> queries(n);
    glFuncs->glGenQueries(n, queries.data());

    glFuncs->glDepthMask(GL_FALSE);
    glFuncs->glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    for (int i = 0; i < n; ++i) {
        glFuncs->glBeginQuery(GL_ANY_SAMPLES_PASSED, queries[i]);
        spheres[i].draw(glFuncs);
        glFuncs->glEndQuery(GL_ANY_SAMPLES_PASSED);
    }

    // Paso 3: leer resultados
    for (int i = 0; i < n; ++i) {
        GLuint result = 0;
        glFuncs->glGetQueryObjectuiv(queries[i], GL_QUERY_RESULT, &result);
        visibilityMask[i] = (result != 0);
    }

    glFuncs->glDeleteQueries(n, queries.data());

    // Restaurar flags
    glFuncs->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glFuncs->glDepthMask(GL_TRUE);
}



std::vector<Sphere> esferas = {
    {{0.0f, 0.0f, -5.0f}, 0.5f},
    {{0.1f, 0.0f, -5.0f}, 0.5f},
    {{-1.0f, 0.0f, -6.0f}, 0.5f}
};

std::vector<bool> visibles;

computeSphereVisibility(esferas, visibles);

for (size_t i = 0; i < esferas.size(); ++i) {
    qDebug() << "Esfera" << i << (visibles[i] ? "VISIBLE" : "OCULTA");
}
