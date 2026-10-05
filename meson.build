project('wurm_explorer', 'cpp',
  version : '0.2.2',
  default_options : ['warning_level=3', 'cpp_std=c++23'])

add_project_arguments('-Wno-sfinae-incomplete', '-DAPP_VERSION="' + meson.project_version() + '"', language: 'cpp')

# Dependencies
qt6 = import('qt6')
qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets', 'Concurrent', 'Test'])
vips_dep = dependency('vips-cpp')
opencv_dep = dependency('opencv4', required: true)
yaml_dep = dependency('yaml-cpp', required: true)
json_dep = dependency('nlohmann_json', required: true)
tesseract_dep = dependency('tesseract', required: true)
omp_dep = dependency('openmp', required: false)

incdir = include_directories('src')

# Source files
sources = [
  'src/main.cpp',
  'src/engine/ZoomMapCanvas.cpp',
  'src/engine/ZoomMapCanvasEvents.cpp',
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
  'src/ui/panels/SailingPanelOverlays.cpp',
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
  'src/core/CliOptions.cpp',
  'src/ui/MainWindowLocate.cpp',
  'src/ui/MainWindowCapture.cpp',
  'src/features/granger/GrangerTraits.cpp',
  'src/features/granger/BreedingEvaluator.cpp',
  'src/features/granger/GrangerStore.cpp',
  'src/features/granger/GrangerPanel.cpp',
  'src/features/granger/TraitSelectionDialog.cpp',
  'src/features/skills/SkillLogParser.cpp',
  'src/features/skills/SkillTracker.cpp',
  'src/features/skills/SkillPathResolver.cpp',
  'src/features/skills/SkillsPanel.cpp',
  'src/ui/dialogs/SettingsDialog.cpp',
  'src/features/tools/ImpCalculator.cpp',
  'src/features/tools/BridgePillarCalculator.cpp',
  'src/features/tools/GrinderEngine.cpp',
  'src/features/tools/DifficultyPresets.cpp',
  'src/features/tools/ImpCalculatorWidget.cpp',
  'src/features/tools/BridgePillarWidget.cpp',
  'src/features/tools/GrinderChartWidget.cpp',
  'src/features/tools/GrinderWidget.cpp',
  'src/features/tools/ToolsPanel.cpp',
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
  'src/ui/panels/SailingPanel.hpp',
  'src/features/granger/GrangerPanel.hpp',
  'src/features/granger/TraitSelectionDialog.hpp',
  'src/features/skills/SkillsPanel.hpp',
  'src/features/skills/SkillTracker.hpp',
  'src/ui/dialogs/SettingsDialog.hpp',
  'src/features/tools/ImpCalculatorWidget.hpp',
  'src/features/tools/BridgePillarWidget.hpp',
  'src/features/tools/GrinderChartWidget.hpp',
  'src/features/tools/GrinderWidget.hpp',
  'src/features/tools/ToolsPanel.hpp',
]



processed = qt6.preprocess(
  moc_headers : moc_headers,
  dependencies: qt6_dep
)

executable('wurm_explorer',
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
test('cv', test_cv, workdir: meson.project_source_root())

test_store = executable('test_store',
  ['tests/test_store.cpp', 'src/models/AnnotationStore.cpp', 'src/models/ArtifactStore.cpp', 'src/models/DrawingStore.cpp'],
  dependencies: [json_dep]
)
test('store', test_store, workdir: meson.project_source_root())

test_ui_src = [
  'tests/test_ui.cpp',
  'src/ui/panels/ArtifactPanel.cpp',
  'src/models/ArtifactStore.cpp',
  'src/features/treasure/TreasurePanel.cpp',
  'src/features/drawing/DrawingPanel.cpp',
  'src/models/DrawingStore.cpp',
  'src/models/AnnotationStore.cpp',
  'src/features/skills/SkillsPanel.cpp',
  'src/features/skills/SkillTracker.cpp',
  'src/features/skills/SkillLogParser.cpp',
  'src/features/skills/SkillPathResolver.cpp',
  'src/features/tools/ImpCalculatorWidget.cpp',
  'src/features/tools/ImpCalculator.cpp',
  'src/ui/dialogs/SettingsDialog.cpp'
]

test_ui_moc = qt6.preprocess(
  moc_sources: ['tests/test_ui.cpp'],
  moc_headers: [
    'src/ui/panels/ArtifactPanel.hpp',
    'src/features/treasure/TreasurePanel.hpp',
    'src/features/drawing/DrawingPanel.hpp',
    'src/features/skills/SkillsPanel.hpp',
    'src/features/skills/SkillTracker.hpp',
    'src/features/tools/ImpCalculatorWidget.hpp',
    'src/ui/dialogs/SettingsDialog.hpp'
  ],
  dependencies: qt6_dep
)

test_ui = executable('test_ui',
  test_ui_src, test_ui_moc,
  dependencies: [qt6_dep, json_dep, opencv_dep]
)
test('ui', test_ui, workdir: meson.project_source_root(), env: ['QT_QPA_PLATFORM=offscreen'])

test_granger = executable('test_granger',
  ['tests/test_granger.cpp',
   'src/features/granger/GrangerTraits.cpp',
   'src/features/granger/BreedingEvaluator.cpp',
   'src/features/granger/GrangerStore.cpp'],
  dependencies: [json_dep],
  include_directories: incdir
)
test('granger', test_granger, workdir: meson.project_source_root())

test_skills_moc = qt6.preprocess(
  moc_headers: ['src/features/skills/SkillTracker.hpp'],
  dependencies: qt6_dep
)

test_skills = executable('test_skills',
  ['tests/test_skills.cpp',
   'src/features/skills/SkillLogParser.cpp',
   'src/features/skills/SkillTracker.cpp',
   'src/features/skills/SkillPathResolver.cpp',
   test_skills_moc],
  dependencies: [qt6_dep, json_dep],
  include_directories: incdir
)
test('skills', test_skills, workdir: meson.project_source_root())

test_sailing_src = [
  'tests/test_sailing.cpp',
  'src/models/ClusterLayout.cpp',
  'src/models/ClusterDrawingStore.cpp',
  'src/models/DrawingStore.cpp',
  'src/models/AnnotationStore.cpp',
  'src/ui/panels/SailingPanel.cpp',
  'src/ui/panels/SailingPanelOverlays.cpp'
]

test_sailing_moc = qt6.preprocess(
  moc_sources: ['tests/test_sailing.cpp'],
  moc_headers: ['src/ui/panels/SailingPanel.hpp'],
  dependencies: qt6_dep
)

test_sailing = executable('test_sailing',
  test_sailing_src, test_sailing_moc,
  dependencies: [qt6_dep, json_dep],
  include_directories: incdir
)
test('sailing', test_sailing, workdir: meson.project_source_root(), env: ['QT_QPA_PLATFORM=offscreen'])

test_tools = executable('test_tools',
  ['tests/test_tools.cpp',
   'src/features/tools/ImpCalculator.cpp',
   'src/features/tools/BridgePillarCalculator.cpp',
   'src/features/tools/GrinderEngine.cpp',
   'src/features/tools/DifficultyPresets.cpp'],
  dependencies: [qt6_dep],
  include_directories: incdir
)
test('tools', test_tools, suite: 'tools', workdir: meson.project_source_root())

test_docs_moc = qt6.preprocess(
  moc_sources: ['tests/test_docs.cpp'],
  dependencies: qt6_dep
)
test_docs = executable('test_docs',
  ['tests/test_docs.cpp', test_docs_moc],
  dependencies: [qt6_dep],
  include_directories: incdir
)
test('docs', test_docs, suite: 'docs', workdir: meson.project_source_root())

test_bridge_pillar_moc = qt6.preprocess(
  moc_sources: ['tests/test_bridge_pillar.cpp'],
  moc_headers: ['src/features/tools/BridgePillarWidget.hpp'],
  dependencies: qt6_dep
)
test_bridge_pillar = executable('test_bridge_pillar',
  ['tests/test_bridge_pillar.cpp',
   'src/features/tools/BridgePillarCalculator.cpp',
   'src/features/tools/BridgePillarWidget.cpp',
   test_bridge_pillar_moc],
  dependencies: [qt6_dep],
  include_directories: incdir
)
test('bridge_pillar', test_bridge_pillar, suite: 'tools', workdir: meson.project_source_root(), env: ['QT_QPA_PLATFORM=offscreen'])

test_cli_moc = qt6.preprocess(
  moc_sources: ['tests/test_cli.cpp'],
  dependencies: qt6_dep
)
test_cli = executable('test_cli',
  ['tests/test_cli.cpp', 'src/core/CliOptions.cpp', test_cli_moc],
  dependencies: [qt6_dep],
  include_directories: incdir
)
test('cli', test_cli, suite: 'cli', workdir: meson.project_source_root())

# Install desktop and icon integration
install_data('assets/icons/wurm_explorer.svg',
  install_dir: get_option('datadir') / 'icons/hicolor/scalable/apps',
  rename: 'wurm_explorer.svg'
)
install_data('assets/icons/wurm_explorer.svg',
  install_dir: get_option('datadir') / 'icons/hicolor/scalable/apps',
  rename: 'wurmexplorer.svg'
)
install_data('wurmexplorer.desktop',
  install_dir: get_option('datadir') / 'applications'
)

