project('wurm_locator', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++23'])

add_project_arguments('-Wno-sfinae-incomplete', language: 'cpp')

# Dependencies
qt6 = import('qt6')
qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets', 'Concurrent', 'Test'])
vips_dep = dependency('vips-cpp')
opencv_dep = dependency('opencv4', required: true)
yaml_dep = dependency('yaml-cpp', required: true)
json_dep = dependency('nlohmann_json', required: true)
tesseract_dep = dependency('tesseract', required: true)
omp_dep = dependency('openmp', required: true)

incdir = include_directories('src')

# Source files
sources = [
  'src/main.cpp',
  'src/engine/ZoomMapCanvas.cpp',
  'src/ui/MainWindow.cpp',
  'src/ui/Theme.cpp',
  'src/ui/components/TopControlBar.cpp',
  'src/ui/components/NavigationBar.cpp',
  'src/features/treasure/TreasurePanel.cpp',
  'src/features/drawing/DrawingPanel.cpp',
  'src/ui/panels/AnnotationPanel.cpp',
  'src/ui/panels/ArtifactPanel.cpp',
  'src/ui/panels/MapDataPanel.cpp',
  'src/ui/panels/SailingPanel.cpp',
  'src/models/ClusterDrawingStore.cpp',
  'src/models/ClusterLayout.cpp',
  'src/models/AnnotationStore.cpp',
  'src/models/ArtifactStore.cpp',
  'src/models/DrawingStore.cpp',
  'src/engine/cv/ImageOps.cpp',
  'src/engine/cv/Extractor.cpp',
  'src/engine/cv/Matcher.cpp',
  'src/engine/cv/Ocr.cpp',
  'src/core/Config.cpp',
  'src/core/MapVariants.cpp',
]

# Qt6 preprocessing (MOC)
moc_headers = [
  'src/engine/ZoomMapCanvas.hpp',
  'src/ui/MainWindow.hpp',
  'src/ui/components/TopControlBar.hpp',
  'src/ui/components/NavigationBar.hpp',
  'src/ui/panels/AnnotationPanel.hpp',
  'src/features/treasure/TreasurePanel.hpp',
  'src/features/drawing/DrawingPanel.hpp',
  'src/ui/panels/ArtifactPanel.hpp',
  'src/ui/panels/MapDataPanel.hpp',
  'src/ui/panels/SailingPanel.hpp'
]

processed = qt6.preprocess(
  moc_headers : moc_headers,
  dependencies: qt6_dep
)

executable('wurm_locator',
  processed,
  sources,
  include_directories: incdir,
  dependencies: [qt6_dep, vips_dep, opencv_dep, yaml_dep, json_dep, tesseract_dep, omp_dep],
  install: true)

# Tests
test_cv = executable('test_cv', 
  ['tests/test_cv.cpp', 'src/engine/cv/ImageOps.cpp', 'src/engine/cv/Extractor.cpp', 'src/engine/cv/Matcher.cpp'],
  dependencies: [opencv_dep, omp_dep]
)
test('cv', test_cv)

test_store = executable('test_store',
  ['tests/test_store.cpp', 'src/models/AnnotationStore.cpp', 'src/models/ArtifactStore.cpp', 'src/models/DrawingStore.cpp'],
  dependencies: [json_dep]
)
test('store', test_store)

test_ui_src = [
  'tests/test_ui.cpp',
  'src/ui/panels/ArtifactPanel.cpp',
  'src/models/ArtifactStore.cpp',
  'src/features/treasure/TreasurePanel.cpp',
  'src/features/drawing/DrawingPanel.cpp',
  'src/models/DrawingStore.cpp',
  'src/models/AnnotationStore.cpp'
]

test_ui_moc = qt6.preprocess(
  moc_sources: ['tests/test_ui.cpp'],
  moc_headers: ['src/ui/panels/ArtifactPanel.hpp', 'src/features/treasure/TreasurePanel.hpp', 'src/features/drawing/DrawingPanel.hpp'],
  dependencies: qt6_dep
)

test_ui = executable('test_ui',
  test_ui_src, test_ui_moc,
  dependencies: [qt6_dep, json_dep, opencv_dep]
)
test('ui', test_ui)
