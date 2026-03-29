// RISC-V RTL自动生成项目构建配置

scalaVersion := "2.13.12"

// Chisel版本
val chiselVersion = "6.0.0"

// 添加Chisel编译器插件
addCompilerPlugin("org.chipsalliance" % "chisel-plugin" % chiselVersion cross CrossVersion.full)

// 项目库依赖
libraryDependencies ++= Seq(
  "org.chipsalliance" %% "chisel" % chiselVersion,
  "edu.berkeley.cs" %% "chiseltest" % "6.0.0" % "test"
)

// 编译选项
scalacOptions ++= Seq(
  "-language:reflectiveCalls",
  "-deprecation",
  "-feature",
  "-Xcheckinit",
  "-P:chiselplugin:genBundleElements"
)

// 项目设置
name := "riscv-rtl-auto-gen"
version := "1.0.0"
organization := "edu.fudan.eda"

// 源代码目录
Compile / scalaSource := baseDirectory.value / "src" / "chisel"
Test / scalaSource := baseDirectory.value / "src" / "test"

// 资源文件
Compile / resourceDirectory := baseDirectory.value / "src" / "resources"
Test / resourceDirectory := baseDirectory.value / "src" / "test-resources"

// 输出目录
target := baseDirectory.value / "target"

// 生成FIRRTL的配置
Compile / mainClass := Some("chisel3.stage.ChiselMain")
Compile / run / mainClass := Some("chisel3.stage.ChiselMain")

// 生成Verilog的配置
Compile / run / fork := true
Compile / run / javaOptions ++= Seq(
  "-Xmx2G",
  "-Xss2M"
)