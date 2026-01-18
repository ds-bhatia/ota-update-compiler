; ModuleID = 'tests/insecure.c'
source_filename = "tests/insecure.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.UpdatePkg = type { i32 }

@current_version = dso_local global i32 5, align 4
@.str = private unnamed_addr constant [33 x i8] c"Installing firmware version: %d\0A\00", align 1
@__const.main.pkg = private unnamed_addr constant %struct.UpdatePkg { i32 6 }, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @verifySignature(%struct.UpdatePkg* noundef %0) #0 {
  %2 = alloca %struct.UpdatePkg*, align 8
  store %struct.UpdatePkg* %0, %struct.UpdatePkg** %2, align 8
  ret i32 1
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @sourceTrusted(%struct.UpdatePkg* noundef %0) #0 {
  %2 = alloca %struct.UpdatePkg*, align 8
  store %struct.UpdatePkg* %0, %struct.UpdatePkg** %2, align 8
  ret i32 1
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @install(%struct.UpdatePkg* noundef %0) #0 {
  %2 = alloca %struct.UpdatePkg*, align 8
  store %struct.UpdatePkg* %0, %struct.UpdatePkg** %2, align 8
  %3 = load %struct.UpdatePkg*, %struct.UpdatePkg** %2, align 8
  %4 = getelementptr inbounds %struct.UpdatePkg, %struct.UpdatePkg* %3, i32 0, i32 0
  %5 = load i32, i32* %4, align 4
  %6 = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([33 x i8], [33 x i8]* @.str, i64 0, i64 0), i32 noundef %5)
  ret void
}

declare i32 @printf(i8* noundef, ...) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @updateFirmware(%struct.UpdatePkg* noundef %0) #0 {
  %2 = alloca %struct.UpdatePkg*, align 8
  store %struct.UpdatePkg* %0, %struct.UpdatePkg** %2, align 8
  %3 = load %struct.UpdatePkg*, %struct.UpdatePkg** %2, align 8
  call void @install(%struct.UpdatePkg* noundef %3)
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.UpdatePkg, align 4
  store i32 0, i32* %1, align 4
  %3 = bitcast %struct.UpdatePkg* %2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %3, i8* align 4 bitcast (%struct.UpdatePkg* @__const.main.pkg to i8*), i64 4, i1 false)
  call void @updateFirmware(%struct.UpdatePkg* noundef %2)
  ret i32 0
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { argmemonly nofree nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 14.0.6"}
