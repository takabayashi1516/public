Attribute VB_Name = "TableConverterModule"
Public Type TempData
    mTemperature As Double
    mName As String
End Type

Public Type PersonalData
    mDate As String
    mData() As TempData
End Type

Public Sub convertTable(inpSheet As Worksheet, outpSheet As Worksheet)
On Error Resume Next
    Const NAME_COL$ = "B"
    Const TEMP_COL$ = "D"
    Const DATE_COL$ = "F"

    Dim persons() As String
    Dim is_exst As Boolean

    Dim pdt() As PersonalData
    With inpSheet
        r% = 2
        Do While (.Range(DATE_COL + Trim(Str(r))).Text <> "")
            strt% = LBound(persons)
            If (Err.Number) Then
                Err.Clear
                ReDim persons(0 To 0)
                persons(0) = .Range(NAME_COL + Trim(Str(r))).Text
            Else
                is_exst = False
                For i% = LBound(persons) To UBound(persons)
                    If (persons(i) = .Range(NAME_COL + Trim(Str(r))).Text) Then
                        is_exst = True
                        Exit For
                    End If
                Next i
                If (Not is_exst) Then
                    ReDim Preserve persons(LBound(persons) To (UBound(persons) + 1))
                    persons(UBound(persons)) = .Range(NAME_COL + Trim(Str(r))).Text
                End If
            End If

            strt = LBound(pdt)
            If (Err.Number) Then
                Err.Clear
                ReDim pdt(0 To 0)
                pdt(0).mDate = .Range(DATE_COL + Trim(Str(r))).Text
                ReDim pdt(0).mData(0 To 0)
                pdt(0).mData(0).mTemperature = Val(.Range(TEMP_COL + Trim(Str(r))).Text)
                pdt(0).mData(0).mName = .Range(NAME_COL + Trim(Str(r))).Text
            Else
                is_exst = False
                For i% = strt To UBound(pdt)
                    If (pdt(i).mDate = .Range(DATE_COL + Trim(Str(r))).Text) Then
                        ReDim Preserve pdt(i).mData(LBound(pdt(i).mData) To (UBound(pdt(i).mData) + 1))
                        pdt(i).mData(UBound(pdt(i).mData)).mTemperature = Val(.Range(TEMP_COL + Trim(Str(r))).Text)
                        pdt(i).mData(UBound(pdt(i).mData)).mName = .Range(NAME_COL + Trim(Str(r))).Text
                        is_exst = True
                        Exit For
                    End If
                Next i
                If (Not is_exst) Then
                    ReDim Preserve pdt(LBound(pdt) To (UBound(pdt) + 1))
                    ReDim pdt(UBound(pdt)).mData(0 To 0)
                    pdt(UBound(pdt)).mDate = .Range(DATE_COL + Trim(Str(r))).Text
                    pdt(UBound(pdt)).mData(0).mTemperature = Val(.Range(TEMP_COL + Trim(Str(r))).Text)
                    pdt(UBound(pdt)).mData(0).mName = .Range(NAME_COL + Trim(Str(r))).Text
                End If
            End If
            r = r + 1
        Loop
    End With

    With outpSheet
        For i% = LBound(persons) To UBound(persons)
            .Cells(i + 2, 1).Value = persons(i)
        Next i
        For i% = LBound(pdt) To UBound(pdt)
            .Cells(1, i + 2).Value = pdt(i).mDate
            For j% = LBound(pdt(i).mData) To UBound(pdt(i).mData)
                For k% = LBound(persons) To UBound(persons)
                    If (pdt(i).mData(j).mName = persons(k)) Then
                        Exit For
                    End If
                Next k
                .Cells(k + 2, i + 2).Value = pdt(i).mData(j).mTemperature
            Next j
        Next i
    End With
End Sub
