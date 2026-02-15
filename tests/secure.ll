; ModuleID = 'tests/secure.c'
source_filename = "tests/secure.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.UpdatePkg = type { i32 }

@current_version = dso_local local_unnamed_addr global i32 5, align 4
@.str = private unnamed_addr constant [33 x i8] c"Installing firmware version: %d\0A\00", align 1
@str = private unnamed_addr constant [22 x i8] c"[LOG] Update accepted\00", align 1
@str.5 = private unnamed_addr constant [41 x i8] c"[LOG] Update rejected: rollback detected\00", align 1

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn
define dso_local i32 @verifySignature(%struct.UpdatePkg* nocapture readnone %0) local_unnamed_addr #0 {
  ret i32 1
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn
define dso_local i32 @sourceTrusted(%struct.UpdatePkg* nocapture readnone %0) local_unnamed_addr #0 {
  ret i32 1
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @install(%struct.UpdatePkg* nocapture noundef readonly %0) local_unnamed_addr #1 {
  %2 = getelementptr inbounds %struct.UpdatePkg, %struct.UpdatePkg* %0, i64 0, i32 0
  %3 = load i32, i32* %2, align 4, !tbaa !5
  %4 = call i32 (i8*, ...) @printf(i8* noundef nonnull dereferenceable(1) getelementptr inbounds ([33 x i8], [33 x i8]* @.str, i64 0, i64 0), i32 noundef %3)
  ret void
}

; Function Attrs: nofree nounwind
declare noundef i32 @printf(i8* nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind uwtable
define dso_local void @updateFirmware(%struct.UpdatePkg* nocapture noundef readonly %0) local_unnamed_addr #1 {
  %2 = getelementptr inbounds %struct.UpdatePkg, %struct.UpdatePkg* %0, i64 0, i32 0
  %3 = load i32, i32* %2, align 4, !tbaa !5
  %4 = load i32, i32* @current_version, align 4, !tbaa !10
  %5 = icmp sgt i32 %3, %4
  br i1 %5, label %8, label %6

6:                                                ; preds = %1
  %7 = call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([41 x i8], [41 x i8]* @str.5, i64 0, i64 0))
  br label %12

8:                                                ; preds = %1
  %9 = call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([22 x i8], [22 x i8]* @str, i64 0, i64 0))
  %10 = load i32, i32* %2, align 4, !tbaa !5
  %11 = call i32 (i8*, ...) @printf(i8* noundef nonnull dereferenceable(1) getelementptr inbounds ([33 x i8], [33 x i8]* @.str, i64 0, i64 0), i32 noundef %10) #4
  br label %12

12:                                               ; preds = %8, %6
  ret void
}

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
  %1 = load i32, i32* @current_version, align 4, !tbaa !10
  %2 = icmp slt i32 %1, 6
  br i1 %2, label %5, label %3

3:                                                ; preds = %0
  %4 = call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([41 x i8], [41 x i8]* @str.5, i64 0, i64 0)) #4
  br label %8

5:                                                ; preds = %0
  %6 = call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([22 x i8], [22 x i8]* @str, i64 0, i64 0)) #4
  %7 = call i32 (i8*, ...) @printf(i8* noundef nonnull dereferenceable(1) getelementptr inbounds ([33 x i8], [33 x i8]* @.str, i64 0, i64 0), i32 noundef 6) #4
  br label %8

8:                                                ; preds = %3, %5
  ret i32 0
}

; Function Attrs: nofree nounwind
declare noundef i32 @puts(i8* nocapture noundef readonly) local_unnamed_addr #3

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nofree nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nofree nounwind }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{!"Ubuntu clang version 14.0.6"}
!5 = !{!6, !7, i64 0}
!6 = !{!"", !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!7, !7, i64 0}
