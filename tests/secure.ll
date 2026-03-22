; ModuleID = 'tests/secure.c'
source_filename = "tests/secure.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.DeviceState = type { i32, i32, i32, [16 x i8] }
%struct.FirmwarePackage = type { i32, [16 x i8], [128 x i8], [32 x i8], [64 x i8], [512 x i8], i32 }

@current_version = dso_local global i32 5, align 4
@.str = private unnamed_addr constant [32 x i8] c"https://updates.vendor.example/\00", align 1
@__const.main.device = private unnamed_addr constant %struct.DeviceState { i32 5, i32 0, i32 1, [16 x i8] c"stable\00\00\00\00\00\00\00\00\00\00" }, align 4
@__const.main.pkg = private unnamed_addr constant %struct.FirmwarePackage { i32 6, [16 x i8] c"stable\00\00\00\00\00\00\00\00\00\00", [128 x i8] c"https://updates.vendor.example/firmware/v6.bin\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [32 x i8] zeroinitializer, [64 x i8] zeroinitializer, [512 x i8] zeroinitializer, i32 512 }, align 4
@.str.1 = private unnamed_addr constant [28 x i8] c"OTA completed successfully\0A\00", align 1
@.str.2 = private unnamed_addr constant [33 x i8] c"OTA failed with status code: %d\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @verifySignature(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  ret i32 1
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @sourceTrusted(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %struct.FirmwarePackage, ptr %3, i32 0, i32 2
  %5 = getelementptr inbounds [128 x i8], ptr %4, i64 0, i64 0
  %6 = call i32 @startsWith(ptr noundef %5, ptr noundef @.str)
  ret i32 %6
}

; Function Attrs: noinline nounwind uwtable
define internal i32 @startsWith(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load ptr, ptr %4, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = call i64 @strlen(ptr noundef %7) #4
  %9 = call i32 @strncmp(ptr noundef %5, ptr noundef %6, i64 noundef %8) #4
  %10 = icmp eq i32 %9, 0
  %11 = zext i1 %10 to i32
  ret i32 %11
}

; Function Attrs: noinline nounwind uwtable
define dso_local void @install(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  ret void
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @updateFirmware(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  %6 = load ptr, ptr %5, align 8
  %7 = call i32 @verifySignature(ptr noundef %6)
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %10, label %9

9:                                                ; preds = %2
  store i32 1, ptr %3, align 4
  br label %40

10:                                               ; preds = %2
  %11 = load ptr, ptr %5, align 8
  %12 = call i32 @sourceTrusted(ptr noundef %11)
  %13 = icmp ne i32 %12, 0
  br i1 %13, label %15, label %14

14:                                               ; preds = %10
  store i32 2, ptr %3, align 4
  br label %40

15:                                               ; preds = %10
  %16 = load ptr, ptr %5, align 8
  %17 = getelementptr inbounds %struct.FirmwarePackage, ptr %16, i32 0, i32 0
  %18 = load i32, ptr %17, align 4
  %19 = load i32, ptr @current_version, align 4
  %20 = icmp sle i32 %18, %19
  br i1 %20, label %21, label %22

21:                                               ; preds = %15
  store i32 3, ptr %3, align 4
  br label %40

22:                                               ; preds = %15
  %23 = load ptr, ptr %5, align 8
  %24 = call i32 @verifyImageHash(ptr noundef %23)
  %25 = icmp ne i32 %24, 0
  br i1 %25, label %27, label %26

26:                                               ; preds = %22
  store i32 4, ptr %3, align 4
  br label %40

27:                                               ; preds = %22
  %28 = load ptr, ptr %5, align 8
  call void @install(ptr noundef %28)
  %29 = load ptr, ptr %5, align 8
  %30 = getelementptr inbounds %struct.FirmwarePackage, ptr %29, i32 0, i32 0
  %31 = load i32, ptr %30, align 4
  %32 = load ptr, ptr %4, align 8
  %33 = getelementptr inbounds %struct.DeviceState, ptr %32, i32 0, i32 0
  store i32 %31, ptr %33, align 4
  %34 = load ptr, ptr %4, align 8
  %35 = getelementptr inbounds %struct.DeviceState, ptr %34, i32 0, i32 1
  %36 = load i32, ptr %35, align 4
  %37 = xor i32 %36, 1
  %38 = load ptr, ptr %4, align 8
  %39 = getelementptr inbounds %struct.DeviceState, ptr %38, i32 0, i32 2
  store i32 %37, ptr %39, align 4
  store i32 0, ptr %3, align 4
  br label %40

40:                                               ; preds = %27, %26, %21, %14, %9
  %41 = load i32, ptr %3, align 4
  ret i32 %41
}

; Function Attrs: noinline nounwind uwtable
define internal i32 @verifyImageHash(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  ret i32 1
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.DeviceState, align 4
  %3 = alloca %struct.FirmwarePackage, align 4
  %4 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %2, ptr align 4 @__const.main.device, i64 28, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %3, ptr align 4 @__const.main.pkg, i64 760, i1 false)
  %5 = call i32 @updateFirmware(ptr noundef %2, ptr noundef %3)
  store i32 %5, ptr %4, align 4
  %6 = load i32, ptr %4, align 4
  call void @printResult(i32 noundef %6)
  %7 = load i32, ptr %4, align 4
  %8 = icmp eq i32 %7, 0
  %9 = zext i1 %8 to i64
  %10 = select i1 %8, i32 0, i32 1
  ret i32 %10
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #1

; Function Attrs: noinline nounwind uwtable
define internal void @printResult(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = load i32, ptr %2, align 4
  %4 = icmp eq i32 %3, 0
  br i1 %4, label %5, label %7

5:                                                ; preds = %1
  %6 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  br label %10

7:                                                ; preds = %1
  %8 = load i32, ptr %2, align 4
  %9 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, i32 noundef %8)
  br label %10

10:                                               ; preds = %7, %5
  ret void
}

; Function Attrs: nounwind willreturn memory(read)
declare i32 @strncmp(ptr noundef, ptr noundef, i64 noundef) #2

; Function Attrs: nounwind willreturn memory(read)
declare i64 @strlen(ptr noundef) #2

declare i32 @printf(ptr noundef, ...) #3

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nounwind willreturn memory(read) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind willreturn memory(read) }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
