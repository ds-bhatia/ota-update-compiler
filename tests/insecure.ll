; ModuleID = 'tests/insecure.c'
source_filename = "tests/insecure.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.FirmwarePackage = type { i32, [128 x i8], [64 x i8], [512 x i8], i32 }

@current_version = dso_local global i32 5, align 4
@.str = private unnamed_addr constant [32 x i8] c"https://updates.vendor.example/\00", align 1
@.str.1 = private unnamed_addr constant [43 x i8] c"[device] flashing firmware v%d (%u bytes)\0A\00", align 1
@.str.2 = private unnamed_addr constant [30 x i8] c"[debug] received OTA from %s\0A\00", align 1
@.str.3 = private unnamed_addr constant [33 x i8] c"[debug] random throttle applied\0A\00", align 1
@.str.4 = private unnamed_addr constant [59 x i8] c"[warn] source was not trusted but image already installed\0A\00", align 1
@.str.5 = private unnamed_addr constant [44 x i8] c"[warn] signature failed after installation\0A\00", align 1
@.str.6 = private unnamed_addr constant [45 x i8] c"[warn] rollback detected after installation\0A\00", align 1
@__const.main.pkg = private unnamed_addr constant %struct.FirmwarePackage { i32 4, [128 x i8] c"http://mirror.local/firmware/latest.bin\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [64 x i8] zeroinitializer, [512 x i8] zeroinitializer, i32 512 }, align 4

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
  %4 = getelementptr inbounds %struct.FirmwarePackage, ptr %3, i32 0, i32 1
  %5 = getelementptr inbounds [128 x i8], ptr %4, i64 0, i64 0
  %6 = call i32 @strncmp(ptr noundef %5, ptr noundef @.str, i64 noundef 31) #5
  %7 = icmp eq i32 %6, 0
  %8 = zext i1 %7 to i32
  ret i32 %8
}

; Function Attrs: nounwind willreturn memory(read)
declare i32 @strncmp(ptr noundef, ptr noundef, i64 noundef) #1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @SHA1(ptr noundef %0, i32 noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  store i32 %1, ptr %4, align 4
  %5 = load ptr, ptr %3, align 8
  %6 = load i32, ptr %4, align 4
  ret i32 1
}

; Function Attrs: noinline nounwind uwtable
define dso_local void @install(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %struct.FirmwarePackage, ptr %3, i32 0, i32 0
  %5 = load i32, ptr %4, align 4
  %6 = load ptr, ptr %2, align 8
  %7 = getelementptr inbounds %struct.FirmwarePackage, ptr %6, i32 0, i32 4
  %8 = load i32, ptr %7, align 4
  %9 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %5, i32 noundef %8)
  ret void
}

declare i32 @printf(ptr noundef, ...) #2

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @updateFirmware(ptr noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  %4 = load ptr, ptr %3, align 8
  %5 = getelementptr inbounds %struct.FirmwarePackage, ptr %4, i32 0, i32 1
  %6 = getelementptr inbounds [128 x i8], ptr %5, i64 0, i64 0
  %7 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, ptr noundef %6)
  %8 = call i32 @rand() #6
  %9 = srem i32 %8, 2
  %10 = icmp eq i32 %9, 0
  br i1 %10, label %11, label %13

11:                                               ; preds = %1
  %12 = call i32 (ptr, ...) @printf(ptr noundef @.str.3)
  br label %13

13:                                               ; preds = %11, %1
  %14 = load ptr, ptr %3, align 8
  %15 = getelementptr inbounds %struct.FirmwarePackage, ptr %14, i32 0, i32 3
  %16 = getelementptr inbounds [512 x i8], ptr %15, i64 0, i64 0
  %17 = load ptr, ptr %3, align 8
  %18 = getelementptr inbounds %struct.FirmwarePackage, ptr %17, i32 0, i32 4
  %19 = load i32, ptr %18, align 4
  %20 = call i32 @SHA1(ptr noundef %16, i32 noundef %19)
  %21 = icmp ne i32 %20, 0
  br i1 %21, label %23, label %22

22:                                               ; preds = %13
  store i32 -1, ptr %2, align 4
  br label %45

23:                                               ; preds = %13
  %24 = load ptr, ptr %3, align 8
  call void @install(ptr noundef %24)
  %25 = load ptr, ptr %3, align 8
  %26 = call i32 @sourceTrusted(ptr noundef %25)
  %27 = icmp ne i32 %26, 0
  br i1 %27, label %30, label %28

28:                                               ; preds = %23
  %29 = call i32 (ptr, ...) @printf(ptr noundef @.str.4)
  br label %30

30:                                               ; preds = %28, %23
  %31 = load ptr, ptr %3, align 8
  %32 = call i32 @verifySignature(ptr noundef %31)
  %33 = icmp ne i32 %32, 0
  br i1 %33, label %36, label %34

34:                                               ; preds = %30
  %35 = call i32 (ptr, ...) @printf(ptr noundef @.str.5)
  br label %36

36:                                               ; preds = %34, %30
  %37 = load ptr, ptr %3, align 8
  %38 = getelementptr inbounds %struct.FirmwarePackage, ptr %37, i32 0, i32 0
  %39 = load i32, ptr %38, align 4
  %40 = load i32, ptr @current_version, align 4
  %41 = icmp sle i32 %39, %40
  br i1 %41, label %42, label %44

42:                                               ; preds = %36
  %43 = call i32 (ptr, ...) @printf(ptr noundef @.str.6)
  br label %44

44:                                               ; preds = %42, %36
  store i32 0, ptr %2, align 4
  br label %45

45:                                               ; preds = %44, %22
  %46 = load i32, ptr %2, align 4
  ret i32 %46
}

; Function Attrs: nounwind
declare i32 @rand() #3

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.FirmwarePackage, align 4
  store i32 0, ptr %1, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %2, ptr align 4 @__const.main.pkg, i64 712, i1 false)
  %3 = call i32 @updateFirmware(ptr noundef %2)
  ret i32 %3
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #4

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind willreturn memory(read) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #5 = { nounwind willreturn memory(read) }
attributes #6 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
