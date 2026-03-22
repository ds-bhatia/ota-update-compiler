; ModuleID = 'tests/insecure_rule_signature_missing.c'
source_filename = "tests/insecure_rule_signature_missing.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.FirmwarePackage = type { i32, [128 x i8], [1024 x i8], i32 }

@current_version = dso_local global i32 5, align 4
@.str = private unnamed_addr constant [32 x i8] c"https://updates.vendor.example/\00", align 1
@__const.main.pkg = private unnamed_addr constant %struct.FirmwarePackage { i32 7, [128 x i8] c"https://updates.vendor.example/release/fw-v7.bin\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00", [1024 x i8] zeroinitializer, i32 1024 }, align 4

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
  %6 = call i32 @strncmp(ptr noundef %5, ptr noundef @.str, i64 noundef 31) #3
  %7 = icmp eq i32 %6, 0
  %8 = zext i1 %7 to i32
  ret i32 %8
}

; Function Attrs: nounwind willreturn memory(read)
declare i32 @strncmp(ptr noundef, ptr noundef, i64 noundef) #1

; Function Attrs: noinline nounwind uwtable
define dso_local void @install(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  ret void
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @updateFirmware(ptr noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  %4 = load ptr, ptr %3, align 8
  %5 = call i32 @sourceTrusted(ptr noundef %4)
  %6 = icmp ne i32 %5, 0
  br i1 %6, label %8, label %7

7:                                                ; preds = %1
  store i32 -1, ptr %2, align 4
  br label %17

8:                                                ; preds = %1
  %9 = load ptr, ptr %3, align 8
  %10 = getelementptr inbounds %struct.FirmwarePackage, ptr %9, i32 0, i32 0
  %11 = load i32, ptr %10, align 4
  %12 = load i32, ptr @current_version, align 4
  %13 = icmp sgt i32 %11, %12
  br i1 %13, label %14, label %16

14:                                               ; preds = %8
  %15 = load ptr, ptr %3, align 8
  call void @install(ptr noundef %15)
  store i32 0, ptr %2, align 4
  br label %17

16:                                               ; preds = %8
  store i32 -1, ptr %2, align 4
  br label %17

17:                                               ; preds = %16, %14, %7
  %18 = load i32, ptr %2, align 4
  ret i32 %18
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.FirmwarePackage, align 4
  store i32 0, ptr %1, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %2, ptr align 4 @__const.main.pkg, i64 1160, i1 false)
  %3 = call i32 @updateFirmware(ptr noundef %2)
  ret i32 %3
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind willreturn memory(read) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { nounwind willreturn memory(read) }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
