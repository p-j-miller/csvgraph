object PlotWindow: TPlotWindow
  Left = 152
  Top = 22
  Width = 1306
  Height = 1041
  HorzScrollBar.Smooth = True
  HorzScrollBar.Tracking = True
  VertScrollBar.Smooth = True
  VertScrollBar.Tracking = True
  Anchors = []
  AutoScroll = True
  BiDiMode = bdLeftToRight
  Caption = 'PlotWindow'
  Color = clBtnFace
  Constraints.MinWidth = 1000
  ParentFont = True
  Menu = MainMenu1
  OldCreateOrder = False
  ParentBiDiMode = False
  Position = poScreenCenter
  Visible = True
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnMouseDown = Image1MouseDown
  OnMouseMove = Image1MouseMove
  OnMouseUp = Image1MouseUp
  OnMouseWheelDown = FormMouseWheelDown
  OnMouseWheelUp = FormMouseWheelUp
  OnResize = FormResize
  DesignSize = (
    1290
    1002)
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 1290
    Height = 980
    TabOrder = 0
    OnMouseDown = Image1MouseDown
    OnMouseMove = Image1MouseMove
    OnMouseUp = Image1MouseUp
    DesignSize = (
      1290
      980)
    object Image1: TImage
      Left = 1
      Top = 1
      Width = 1288
      Height = 978
      Align = alClient
      OnMouseDown = Image1MouseDown
      OnMouseMove = Image1MouseMove
      OnMouseUp = Image1MouseUp
      ExplicitWidth = 1298
    end
    object Shape1: TShape
      Left = 178
      Top = 80
      Width = 65
      Height = 65
      Brush.Style = bsClear
      Pen.Mode = pmNot
      Pen.Style = psDot
    end
    object Label1: TLabel
      Left = 450
      Top = 943
      Width = 40
      Height = 13
      Anchors = [akLeft, akBottom]
      Caption = 'Position:'
      Color = clNone
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clYellow
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
      Transparent = True
    end
    object Label2: TLabel
      Left = 498
      Top = 943
      Width = 21
      Height = 13
      Anchors = [akLeft, akBottom]
      Caption = '(a,b)'
      Color = clNone
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clYellow
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
      Transparent = True
    end
    object BitBtn5: TBitBtn
      Left = 10
      Top = 772
      Width = 30
      Height = 30
      Action = ShiftYPlus
      Anchors = [akLeft, akBottom]
      Caption = '&'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000120B0000120B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333000333
        3333333333777F33333333333309033333333333337F7F333333333333090333
        33333333337F7F33333333333309033333333333337F7F333333333333090333
        33333333337F7F33333333333309033333333333FF7F7FFFF333333000090000
        3333333777737777F333333099999990333333373F3333373333333309999903
        333333337F33337F33333333099999033333333373F333733333333330999033
        3333333337F337F3333333333099903333333333373F37333333333333090333
        33333333337F7F33333333333309033333333333337373333333333333303333
        333333333337F333333333333330333333333333333733333333}
      NumGlyphs = 2
      TabOrder = 0
    end
    object BitBtn4: TBitBtn
      Left = 10
      Top = 812
      Width = 30
      Height = 30
      Action = ZoomInY
      Anchors = [akLeft, akBottom]
      Caption = '&'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000130B0000130B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
        33033333333333333F7F3333333333333000333333333333F777333333333333
        000333333333333F777333333333333000333333333333F77733333333333300
        033333333FFF3F777333333700073B703333333F7773F77733333307777700B3
        33333377333777733333307F8F8F7033333337F333F337F3333377F8F9F8F773
        3333373337F3373F3333078F898F870333337F33F7FFF37F333307F99999F703
        33337F377777337F3333078F898F8703333373F337F33373333377F8F9F8F773
        333337F3373337F33333307F8F8F70333333373FF333F7333333330777770333
        333333773FF77333333333370007333333333333777333333333}
      NumGlyphs = 2
      TabOrder = 1
    end
    object BitBtn3: TBitBtn
      Left = 10
      Top = 854
      Width = 30
      Height = 30
      Action = ZoomOutY
      Anchors = [akLeft, akBottom]
      Caption = '&'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000130B0000130B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
        33033333333333333F7F3333333333333000333333333333F777333333333333
        000333333333333F777333333333333000333333333333F77733333333333300
        033333333FFF3F777333333700073B703333333F7773F77733333307777700B3
        333333773337777333333078F8F87033333337F3333337F33333778F8F8F8773
        333337333333373F333307F8F8F8F70333337F33FFFFF37F3333078999998703
        33337F377777337F333307F8F8F8F703333373F3333333733333778F8F8F8773
        333337F3333337F333333078F8F870333333373FF333F7333333330777770333
        333333773FF77333333333370007333333333333777333333333}
      NumGlyphs = 2
      TabOrder = 2
    end
    object BitBtn2: TBitBtn
      Left = 10
      Top = 894
      Width = 30
      Height = 30
      Action = ShiftYMinus
      Anchors = [akLeft, akBottom]
      Caption = '&'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000120B0000120B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333303333
        333333333337F33333333333333033333333333333373F333333333333090333
        33333333337F7F33333333333309033333333333337373F33333333330999033
        3333333337F337F33333333330999033333333333733373F3333333309999903
        333333337F33337F33333333099999033333333373333373F333333099999990
        33333337FFFF3FF7F33333300009000033333337777F77773333333333090333
        33333333337F7F33333333333309033333333333337F7F333333333333090333
        33333333337F7F33333333333309033333333333337F7F333333333333090333
        33333333337F7F33333333333300033333333333337773333333}
      NumGlyphs = 2
      TabOrder = 3
    end
    object BitBtn6: TBitBtn
      Left = 50
      Top = 934
      Width = 30
      Height = 30
      Action = ShiftXMinus
      Anchors = [akLeft, akBottom]
      Caption = '&'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000120B0000120B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
        3333333333333333333333333333333333333333333333333333333333333333
        3333333333333FF3333333333333003333333333333F77F33333333333009033
        333333333F7737F333333333009990333333333F773337FFFFFF330099999000
        00003F773333377777770099999999999990773FF33333FFFFF7330099999000
        000033773FF33777777733330099903333333333773FF7F33333333333009033
        33333333337737F3333333333333003333333333333377333333333333333333
        3333333333333333333333333333333333333333333333333333333333333333
        3333333333333333333333333333333333333333333333333333}
      NumGlyphs = 2
      TabOrder = 4
    end
    object BitBtn7: TBitBtn
      Left = 90
      Top = 934
      Width = 30
      Height = 30
      Action = ZoomOutX
      Anchors = [akLeft, akBottom]
      Caption = '&'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000130B0000130B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
        33033333333333333F7F3333333333333000333333333333F777333333333333
        000333333333333F777333333333333000333333333333F77733333333333300
        033333333FFF3F777333333700073B703333333F7773F77733333307777700B3
        333333773337777333333078F8F87033333337F3333337F33333778F8F8F8773
        333337333333373F333307F8F8F8F70333337F33FFFFF37F3333078999998703
        33337F377777337F333307F8F8F8F703333373F3333333733333778F8F8F8773
        333337F3333337F333333078F8F870333333373FF333F7333333330777770333
        333333773FF77333333333370007333333333333777333333333}
      NumGlyphs = 2
      TabOrder = 5
    end
    object BitBtn8: TBitBtn
      Left = 130
      Top = 934
      Width = 30
      Height = 30
      Action = ZoomInX
      Anchors = [akLeft, akBottom]
      Caption = '&'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000130B0000130B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
        33033333333333333F7F3333333333333000333333333333F777333333333333
        000333333333333F777333333333333000333333333333F77733333333333300
        033333333FFF3F777333333700073B703333333F7773F77733333307777700B3
        33333377333777733333307F8F8F7033333337F333F337F3333377F8F9F8F773
        3333373337F3373F3333078F898F870333337F33F7FFF37F333307F99999F703
        33337F377777337F3333078F898F8703333373F337F33373333377F8F9F8F773
        333337F3373337F33333307F8F8F70333333373FF333F7333333330777770333
        333333773FF77333333333370007333333333333777333333333}
      NumGlyphs = 2
      TabOrder = 6
    end
    object BitBtn9: TBitBtn
      Left = 170
      Top = 934
      Width = 30
      Height = 30
      Action = ShiftXPlus
      Anchors = [akLeft, akBottom]
      Caption = '&'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000120B0000120B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
        3333333333333333333333333333333333333333333333333333333333333333
        3333333333333333333333333333333333333333333FF3333333333333003333
        3333333333773FF3333333333309003333333333337F773FF333333333099900
        33333FFFFF7F33773FF30000000999990033777777733333773F099999999999
        99007FFFFFFF33333F7700000009999900337777777F333F7733333333099900
        33333333337F3F77333333333309003333333333337F77333333333333003333
        3333333333773333333333333333333333333333333333333333333333333333
        3333333333333333333333333333333333333333333333333333}
      NumGlyphs = 2
      TabOrder = 7
    end
    object BitBtn10: TBitBtn
      Left = 10
      Top = 934
      Width = 30
      Height = 30
      Action = AutoScale
      Anchors = [akLeft, akBottom]
      Caption = '&'
      Glyph.Data = {
        F6000000424DF600000000000000760000002800000010000000100000000100
        0400000000008000000000000000000000001000000000000000000000000000
        80000080000000808000800000008000800080800000C0C0C000808080000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00777777777777
        7777070700077077000700070707707707070707070770770707000707070007
        0007777777777777777700000777777777777000777777777777770777777777
        7777000000009000000997777999797777977997977777997977000900000000
        9000770777777777777770007777777777770000077777777777}
      TabOrder = 8
    end
    object Edit_title: TEdit
      Left = 368
      Top = 8
      Width = 649
      Height = 21
      BorderStyle = bsNone
      Color = clMenuText
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindow
      Font.Height = -16
      Font.Name = 'Arial'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 10
      Text = 'Title'
      OnChange = Edit_titleChange
    end
    object ButtonSave: TBitBtn
      Left = 232
      Top = 934
      Width = 70
      Height = 30
      Anchors = [akLeft, akBottom]
      Caption = 'Plot SaveAs'
      TabOrder = 9
      OnClick = ButtonSave1Click
    end
    object ButtonPlotToClipboard: TBitBtn
      Left = 312
      Top = 934
      Width = 95
      Height = 30
      Anchors = [akLeft, akBottom]
      Caption = 'Plot To Clipboard'
      TabOrder = 11
      OnClick = Button_PlotToClipboard1Click
    end
  end
  object Panel2: TScrollBox
    Left = 975
    Top = -1
    Width = 295
    Height = 980
    HorzScrollBar.Smooth = True
    HorzScrollBar.Tracking = True
    VertScrollBar.Smooth = True
    VertScrollBar.Tracking = True
    Anchors = [akTop, akRight]
    AutoScroll = False
    AutoSize = True
    DockSite = True
    DragKind = dkDock
    DragMode = dmAutomatic
    TabOrder = 1
    OnEndDock = Panel2EndDock
    OnResize = Panel2Resize
    DesignSize = (
      291
      976)
    object Label3: TLabel
      Left = 15
      Top = 487
      Width = 42
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'X column'
    end
    object Label4: TLabel
      Left = 12
      Top = 655
      Width = 42
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Y column'
    end
    object Label5: TLabel
      Left = 4
      Top = 252
      Width = 61
      Height = 16
      Anchors = [akTop, akRight]
      Caption = 'Axis Titles'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
    end
    object Label6: TLabel
      Left = 28
      Top = 295
      Width = 25
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Y (^)'
    end
    object Label7: TLabel
      Left = 28
      Top = 319
      Width = 29
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'X (->)'
    end
    object Label8: TLabel
      Left = 12
      Top = 351
      Width = 25
      Height = 16
      Anchors = [akTop, akRight]
      Caption = 'File:'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
    end
    object Label9: TLabel
      Left = 157
      Top = 487
      Width = 38
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'X offset'
    end
    object Label10: TLabel
      Left = 166
      Top = 859
      Width = 45
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'filter time'
    end
    object Label11: TLabel
      Left = 28
      Top = 274
      Width = 44
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Font Size'
    end
    object Label12: TLabel
      Left = 9
      Top = 814
      Width = 107
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Processing of Y values'
    end
    object Label13: TLabel
      Left = 166
      Top = 876
      Width = 75
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'constant (Secs)'
    end
    object Label14: TLabel
      Left = 164
      Top = 898
      Width = 79
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Polynomial order'
    end
    object Label15: TLabel
      Left = 184
      Top = 378
      Width = 43
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Skip lines'
    end
    object Label16: TLabel
      Left = 189
      Top = 397
      Width = 88
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'before csv header'
    end
    object CheckBox1: TCheckBox
      Left = 20
      Top = 231
      Width = 97
      Height = 17
      Anchors = [akTop, akRight]
      Caption = 'Show Grids'
      Checked = True
      State = cbChecked
      TabOrder = 11
      OnClick = ReDrawExecute
    end
    object RadioGroup2: TRadioGroup
      Left = 20
      Top = 95
      Width = 121
      Height = 105
      Anchors = [akTop, akRight]
      Caption = 'Data Point Style'
      ItemIndex = 0
      Items.Strings = (
        'Circle'
        'Square'
        'Triangle up'
        'Triangle down')
      TabOrder = 14
    end
    object RadioGroup3: TRadioGroup
      Left = 20
      Top = 0
      Width = 121
      Height = 89
      Hint = 
        'Style of graph. Use points when the x axis data is not monotonic' +
        '.'
      Anchors = [akTop, akRight]
      Caption = 'Plot Style'
      ItemIndex = 2
      Items.Strings = (
        'Points'
        'Lines'
        'Points + Lines')
      ParentShowHint = False
      ShowHint = True
      TabOrder = 19
    end
    object CheckBox3: TCheckBox
      Left = 20
      Top = 207
      Width = 105
      Height = 17
      Anchors = [akTop, akRight]
      Caption = 'Filled Data Points'
      TabOrder = 20
    end
    object Edit_xcol: TEdit
      Left = 74
      Top = 479
      Width = 67
      Height = 21
      Hint = 
        'Either select from treeview below or enter an integer column num' +
        'ber'
      Anchors = [akTop, akRight]
      ParentShowHint = False
      ShowHint = True
      TabOrder = 6
      Text = '1'
      OnChange = Edit_xcolChange
    end
    object Edit_ycol: TEdit
      Left = 74
      Top = 647
      Width = 214
      Height = 21
      Hint = 
        'Either select from treeview below or enter an integer or enter a' +
        'n expression using $1.. as columns. Multiple items can be separa' +
        'ted by commas.'
      Anchors = [akTop, akRight]
      Constraints.MinWidth = 214
      ParentShowHint = False
      ShowHint = True
      TabOrder = 8
      Text = '2'
      OnChange = Edit_ycolChange
    end
    object StatusText: TStaticText
      Left = 7
      Top = 960
      Width = 281
      Height = 16
      Anchors = [akTop, akRight]
      AutoSize = False
      Caption = 'No filename set'
      Constraints.MaxWidth = 281
      TabOrder = 15
    end
    object Edit_y: TEdit
      Left = 60
      Top = 295
      Width = 231
      Height = 21
      Hint = 'Vertical (y) axis title - use \n to add a newline'
      Anchors = [akTop, akRight]
      Constraints.MinWidth = 231
      ParentShowHint = False
      ShowHint = True
      TabOrder = 10
      Text = 'Vertical axis title'
      OnChange = Edit_yChange
    end
    object Edit_x: TEdit
      Left = 60
      Top = 319
      Width = 231
      Height = 21
      Hint = 'Horizontal (x) axis title'
      Anchors = [akTop, akRight]
      Constraints.MinWidth = 231
      ParentShowHint = False
      ShowHint = True
      TabOrder = 9
      Text = 'Horizontal axis title'
      OnChange = Edit_xChange
    end
    object StaticText_filename: TEdit
      Left = 41
      Top = 351
      Width = 250
      Height = 18
      Hint = 'Press '#39'Set Filename'#39' Button to  select file'
      Anchors = [akTop, akRight]
      AutoSize = False
      BorderStyle = bsNone
      Color = clMenu
      Constraints.MaxWidth = 250
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -13
      Font.Name = 'Myanmar Text'
      Font.Style = [fsBold]
      ParentFont = False
      ParentShowHint = False
      ReadOnly = True
      ShowHint = False
      TabOrder = 12
      Text = 'Not Set'
    end
    object Xcol_type: TRadioGroup
      Left = 20
      Top = 375
      Width = 153
      Height = 97
      Hint = 'Select data source for the x-axis'
      Anchors = [akTop, akRight]
      Caption = 'X axis data source'
      ItemIndex = 2
      Items.Strings = (
        'Line number in file'
        'Time (h:m:s) in X column'
        'Value in X column'
        'Date/Time in X column')
      ParentShowHint = False
      ShowHint = True
      TabOrder = 5
    end
    object Edit_Xoffset: TEdit
      Left = 214
      Top = 479
      Width = 60
      Height = 21
      Hint = 
        'This value will be added to the x values of the data added whene' +
        'ver <Add Trace> is pressed or to the last trace plotted if  more' +
        ' than one'
      Anchors = [akTop, akRight]
      ParentShowHint = False
      ShowHint = True
      TabOrder = 7
      Text = '0'
      OnChange = Edit_XoffsetChange
    end
    object Edit_median_len: TEdit
      Left = 242
      Top = 868
      Width = 46
      Height = 21
      Hint = 
        'Time that median will be taken over (same units as x-axis). 0 me' +
        'ans no filtering'
      Anchors = [akTop, akRight]
      AutoSize = False
      ParentShowHint = False
      ShowHint = True
      TabOrder = 4
      Text = '0'
    end
    object FilterType: TListBox
      Left = 0
      Top = 833
      Width = 160
      Height = 93
      Hint = 
        'Select filtering required - you may also need to set the filter ' +
        'time constant or polynomial order'
      Anchors = [akTop, akRight]
      ItemHeight = 13
      Items.Strings = (
        'None'
        'Recursive Median Filter'
        'Standard Median Filter'
        'Linear Filter order:'
        'Lin.regression: y=mx'
        'Lin. regression: y=mx+c'
        'GMR regression: y=mx+c'
        'min abs err: y=mx+c'
        'min rel err: y=mx+c'
        'Log: y=m*log(x)+c'
        'Exponential: y=c*exp(mx)'
        'Power: y=c*x^m'
        'Recip: y=m/x+c'
        'y=1/(mx+c)'
        'Hyperbolic: y=x/(m+c*x)'
        'sqrt: y=m*sqrt(x)+c'
        'y=m*x*log2(x)+c'
        'y=a*x+b*sqrt(x)+c'
        'y=a+bx^0.5+cx+dx^1.5'
        'y=(a+bx)/(1+cx)'
        'y=(a+bx+cx2)/(1+dx+ex2)'
        'Polynomial fit order:'
        'Poly in sqrt(x) order:'
        'rational (poly/poly) order:'
        'Derivative (dy/dx)'
        'Integral (y dx)'
        'FFT returns |magnitude|'
        'FFT returns dBV'
        'FFT Hanning win |mag|'
        'FFT Hanning win dBV')
      ParentShowHint = False
      ShowHint = True
      TabOrder = 3
    end
    object CSpinEdit_Fontsize: TCSpinEdit
      Left = 84
      Top = 271
      Width = 50
      Height = 22
      Hint = 'Font size for Titles (typically 12 point)'
      Anchors = [akTop, akRight]
      MaxValue = 14
      MinValue = 8
      ParentShowHint = False
      ShowHint = True
      TabOrder = 13
      Value = 12
      OnChange = CSpinEdit_FontsizeChange
    end
    object ListBoxY: TListBox
      Left = 4
      Top = 671
      Width = 270
      Height = 137
      Hint = 
        'Select (or multiselect) columns to be added when <Add Trace> but' +
        'ton is pressed'
      Anchors = [akTop, akRight]
      Constraints.MinWidth = 270
      ItemHeight = 13
      MultiSelect = True
      ParentShowHint = False
      ShowHint = True
      TabOrder = 16
      OnClick = ListBoxYClick
    end
    object ListBoxX: TListBox
      Left = 4
      Top = 506
      Width = 270
      Height = 137
      Hint = 
        'Select the x (horizontal) axis values (or enter in X column abov' +
        'e)'
      Anchors = [akTop, akRight]
      Constraints.MinWidth = 270
      ItemHeight = 13
      ParentShowHint = False
      ShowHint = True
      TabOrder = 18
      OnClick = ListBoxXClick
    end
    object CheckBox_Compress: TCheckBox
      Left = 129
      Top = 809
      Width = 161
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Compress identical Y values'
      TabOrder = 17
    end
    object Button_Filename: TBitBtn
      Left = 5
      Top = 929
      Width = 89
      Height = 25
      Hint = 'Select csv filename to graph'
      Anchors = [akTop, akRight]
      Caption = 'Set Filename'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 0
      OnClick = Button_Filename1Click
    end
    object Button_add_trace: TBitBtn
      Left = 100
      Top = 929
      Width = 89
      Height = 25
      Hint = 'Add trace "Y column vs X column"'
      Anchors = [akTop, akRight]
      Caption = 'Add Trace(s)'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 1
      OnClick = Button_add_trace1Click
    end
    object Button_clear_all_traces: TBitBtn
      Left = 199
      Top = 929
      Width = 89
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Clear All Traces'
      TabOrder = 2
      OnClick = Button_clear_all_traces1Click
    end
    object BitBtn_set_colour: TBitBtn
      Left = 158
      Top = 120
      Width = 113
      Height = 25
      Hint = 'Press to set the colour of the next added trace'
      Anchors = [akTop, akRight]
      Caption = 'Set next trace colour'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 21
      OnClick = BitBtn_set_colourClick
    end
    object Polyorder: TEdit
      Left = 242
      Top = 895
      Width = 46
      Height = 21
      Anchors = [akTop, akRight]
      NumbersOnly = True
      TabOrder = 22
      Text = '2'
    end
    object Edit_skip_lines: TEdit
      Left = 233
      Top = 375
      Width = 41
      Height = 21
      Hint = 'Enter number of lines to skip in csv file before  header line'
      Anchors = [akTop, akRight]
      NumbersOnly = True
      ParentShowHint = False
      ShowHint = True
      TabOrder = 23
      Text = '0'
    end
    object Time_from0: TCheckBox
      Left = 179
      Top = 416
      Width = 109
      Height = 17
      Hint = 
        'Tick this box to start time from zero (based on the first date/t' +
        'ime in the csv file)'
      Anchors = [akTop, akRight]
      Caption = 'Start time from 0'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 24
      OnClick = Time_from0Click
    end
    object Date_time_fmt: TEdit
      Left = 179
      Top = 452
      Width = 109
      Height = 21
      Hint = 'Enter date/time format eg %d/%m/%y %H:%M:%S.%f'
      Anchors = [akTop, akRight]
      ParentShowHint = False
      ShowHint = True
      TabOrder = 25
      Text = '%d-%b-%y %H:%M:%S.%f'
      OnChange = Edit_xcolChange
    end
  end
  object ActionList1: TActionList
    Images = ImageList1
    Left = 304
    Top = 72
    object ZoomInX: TAction
      Category = 'Graph'
      Caption = 'ZoomInX'
      OnExecute = ZoomInXExecute
    end
    object ZoomOutX: TAction
      Category = 'Graph'
      Caption = 'ZoomOutX'
      OnExecute = ZoomOutXExecute
    end
    object ZoomInY: TAction
      Category = 'Graph'
      Caption = 'ZoomInY'
      OnExecute = ZoomInYExecute
    end
    object ZoomOutY: TAction
      Category = 'Graph'
      Caption = 'ZoomOutY'
      OnExecute = ZoomOutYExecute
    end
    object ShiftXPlus: TAction
      Category = 'Graph'
      Caption = 'ShiftXPlus'
      OnExecute = ShiftXPlusExecute
    end
    object ShiftXMinus: TAction
      Category = 'Graph'
      Caption = 'ShiftXMinus'
      OnExecute = ShiftXMinusExecute
    end
    object ShiftYPlus: TAction
      Category = 'Graph'
      Caption = 'ShiftYPlus'
      OnExecute = ShiftYPlusExecute
    end
    object ShiftYMinus: TAction
      Category = 'Graph'
      Caption = 'ShiftYMinus'
      OnExecute = ShiftYMinusExecute
    end
    object Resize: TAction
      Category = 'Graph'
      Caption = 'Resize'
      ImageIndex = 0
      OnExecute = ResizeExecute
    end
    object AutoScale: TAction
      Category = 'Graph'
      Caption = 'AutoScale'
      ImageIndex = 9
      OnExecute = AutoScaleExecute
    end
    object ReDraw: TAction
      Category = 'Graph'
      Caption = 'ReDraw'
      OnExecute = ReDrawExecute
    end
    object Action1: TAction
      Caption = 'Action1'
      OnExecute = Action1Execute
    end
  end
  object ImageList1: TImageList
    Left = 304
    Top = 32
    Bitmap = {
      494C01010A000E00100210001000FFFFFFFFFF10FFFFFFFFFFFFFFFF424D3600
      0000000000003600000028000000400000003000000001002000000000000030
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000000000000000BDBDBD0000000000BDBD
      BD00000000000000000000000000BDBDBD00BDBDBD0000000000BDBDBD00BDBD
      BD00000000000000000000000000BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000000000000000000000000000BDBD
      BD0000000000BDBDBD0000000000BDBDBD00BDBDBD0000000000BDBDBD00BDBD
      BD0000000000BDBDBD0000000000BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000000000000000BDBDBD0000000000BDBD
      BD0000000000BDBDBD0000000000BDBDBD00BDBDBD0000000000BDBDBD00BDBD
      BD0000000000BDBDBD0000000000BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000000000000000000000000000BDBD
      BD0000000000BDBDBD0000000000BDBDBD00000000000000000000000000BDBD
      BD00000000000000000000000000BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000FF000000FF000000FF00000000000000000000000000
      000000000000000000000000000000000000BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000FF0000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000BDBDBD0000000000000000000000
      0000BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000000000007B7B7B0000FFFF007B7B
      7B0000000000000000000000000000000000BDBDBD00BDBDBD0000000000BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      FF000000FF000000FF0000000000000000000000000000FFFF0000FFFF0000FF
      FF00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000FF0000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      FF000000FF000000FF000000000000000000000000007B7B7B0000FFFF007B7B
      7B00000000000000000000000000000000000000FF00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD000000FF000000FF000000FF00BDBDBD000000FF00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD000000FF00BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      FF000000FF000000FF0000000000000000000000000000000000000000000000
      000000000000000000000000000000000000BDBDBD000000FF000000FF00BDBD
      BD000000FF00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD000000FF000000
      FF00BDBDBD000000FF00BDBDBD00BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      FF00000000000000000000000000000000000000000000000000000000000000
      00000000FF000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000BDBDBD00BDBDBD0000000000BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000BDBDBD0000000000000000000000
      0000BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000000000000000BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000000000000000BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBDBD00BDBD
      BD00BDBDBD00BDBDBD00BDBDBD00000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000FF0000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000000000000000FFFFFF00BDBDBD00FFFF
      FF00BDBDBD00FFFFFF00BDBDBD00FFFFFF00BDBDBD00FFFFFF00BDBDBD00FFFF
      FF00BDBDBD00FFFFFF00BDBDBD00000000000000000000000000000000000000
      000000000000000000000000FF000000FF000000FF0000000000000000000000
      0000000000000000000000000000000000000000000000000000000000007B7B
      7B000000000000000000000000007B7B7B000000000000FFFF007B7B7B000000
      0000000000000000000000000000000000000000000000000000000000007B7B
      7B000000000000000000000000007B7B7B000000000000FFFF007B7B7B000000
      00000000000000000000000000000000000000000000BDBDBD00FFFFFF00BDBD
      BD00FFFFFF00BDBDBD00FFFFFF00BDBDBD00FFFFFF00BDBDBD00FFFFFF00BDBD
      BD00FFFFFF000000FF00FFFFFF00000000000000000000000000000000000000
      00000000FF000000FF000000FF000000FF000000FF0000000000000000000000
      0000000000000000000000000000000000000000000000000000000000007B7B
      7B007B7B7B007B7B7B007B7B7B007B7B7B00000000000000000000FFFF000000
      0000000000000000000000000000000000000000000000000000000000007B7B
      7B007B7B7B007B7B7B007B7B7B007B7B7B00000000000000000000FFFF000000
      00000000000000000000000000000000000000000000FFFFFF00BDBDBD00FFFF
      FF00BDBDBD00FFFFFF00BDBDBD00FFFFFF00BDBDBD00FFFFFF00BDBDBD00FFFF
      FF00BDBDBD00FFFFFF00BDBDBD000000000000000000000000000000FF000000
      FF000000FF000000FF000000FF000000FF000000FF000000FF000000FF000000
      FF000000FF000000FF000000FF000000000000000000000000007B7B7B00FFFF
      FF00BDBDBD00FFFFFF00BDBDBD00FFFFFF007B7B7B0000000000000000000000
      00000000000000000000000000000000000000000000000000007B7B7B00BDBD
      BD00FFFFFF00BDBDBD00FFFFFF00BDBDBD007B7B7B0000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000FF000000FF000000FF000000FF000000FF0000000000000000000000
      0000000000000000000000000000000000007B7B7B007B7B7B00FFFFFF00BDBD
      BD00FFFFFF000000FF00FFFFFF00BDBDBD00FFFFFF007B7B7B007B7B7B000000
      0000000000000000000000000000000000007B7B7B007B7B7B00BDBDBD00FFFF
      FF00BDBDBD00FFFFFF00BDBDBD00FFFFFF00BDBDBD007B7B7B007B7B7B000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFF
      FF00000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000FF000000FF000000FF0000000000000000000000
      000000000000000000000000000000000000000000007B7B7B00BDBDBD00FFFF
      FF00BDBDBD000000FF00BDBDBD00FFFFFF00BDBDBD007B7B7B00000000000000
      000000000000000000000000000000000000000000007B7B7B00FFFFFF00BDBD
      BD00FFFFFF00BDBDBD00FFFFFF00BDBDBD00FFFFFF007B7B7B00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000FFFFFF0000000000000000000000000000000000FFFFFF0000000000FFFF
      FF00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000FF0000000000000000000000
      000000000000000000000000000000000000000000007B7B7B00FFFFFF000000
      FF000000FF000000FF000000FF000000FF00FFFFFF007B7B7B00000000000000
      000000000000000000000000000000000000000000007B7B7B00BDBDBD000000
      FF000000FF000000FF000000FF000000FF00BDBDBD007B7B7B00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFF
      FF00000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000000000007B7B7B00BDBDBD00FFFF
      FF00BDBDBD000000FF00BDBDBD00FFFFFF00BDBDBD007B7B7B00000000000000
      000000000000000000000000000000000000000000007B7B7B00FFFFFF00BDBD
      BD00FFFFFF00BDBDBD00FFFFFF00BDBDBD00FFFFFF007B7B7B00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000FFFFFF000000000000000000FFFFFF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000007B7B7B007B7B7B00FFFFFF00BDBD
      BD00FFFFFF000000FF00FFFFFF00BDBDBD00FFFFFF007B7B7B007B7B7B000000
      0000000000000000000000000000000000007B7B7B007B7B7B00BDBDBD00FFFF
      FF00BDBDBD00FFFFFF00BDBDBD00FFFFFF00BDBDBD007B7B7B007B7B7B000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000FFFFFF00FFFFFF00FFFFFF00FFFFFF0000000000FFFFFF00FFFFFF000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000000000000000000000007B7B7B00FFFF
      FF00BDBDBD00FFFFFF00BDBDBD00FFFFFF007B7B7B0000000000000000000000
      00000000000000000000000000000000000000000000000000007B7B7B00BDBD
      BD00FFFFFF00BDBDBD00FFFFFF00BDBDBD007B7B7B0000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000FFFFFF0000000000BDBDBD00FFFFFF0000000000FFFFFF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000007B7B
      7B007B7B7B007B7B7B007B7B7B007B7B7B000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000007B7B
      7B007B7B7B007B7B7B007B7B7B007B7B7B000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000FFFFFF00FFFFFF00FFFFFF00FFFFFF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000007B7B
      7B000000000000000000000000007B7B7B000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000007B7B
      7B000000000000000000000000007B7B7B000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000FF000000FF000000FF000000FF000000FF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      FF000000FF000000FF000000FF000000FF000000FF000000FF000000FF000000
      FF00000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      00000000000000000000000000000000000000000000000000000000FF000000
      FF000000000000000000000000000000000000000000000000000000FF000000
      FF000000FF000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      000000000000000000000000000000000000000000000000FF000000FF000000
      FF000000FF000000000000000000000000000000000000000000000000000000
      FF000000FF000000FF0000000000000000000000000000000000000000000000
      000000000000000000000000FF000000FF000000FF0000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      000000000000000000000000000000000000000000000000FF00000000000000
      FF000000FF000000FF0000000000000000000000000000000000000000000000
      00000000FF000000FF0000000000000000000000000000000000000000000000
      000000000000000000000000FF000000FF000000FF0000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000FF000000FF00000000000000
      00000000FF000000FF000000FF00000000000000000000000000000000000000
      0000000000000000FF000000FF00000000000000000000000000000000000000
      0000000000000000FF000000FF000000FF000000FF000000FF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000FF000000FF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000FF000000FF00000000000000
      0000000000000000FF000000FF000000FF000000000000000000000000000000
      0000000000000000FF000000FF00000000000000000000000000000000000000
      0000000000000000FF000000FF000000FF000000FF000000FF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000FF000000FF000000FF000000
      FF00000000000000000000000000000000000000000000000000000000000000
      00000000FF000000FF000000FF000000FF000000FF000000FF000000FF000000
      0000000000000000000000000000000000000000FF000000FF00000000000000
      000000000000000000000000FF000000FF000000FF0000000000000000000000
      0000000000000000FF000000FF00000000000000000000000000000000000000
      00000000FF000000FF000000FF000000FF000000FF000000FF000000FF000000
      000000000000000000000000000000000000000000000000FF000000FF000000
      FF000000FF000000FF000000FF000000FF000000FF000000FF000000FF000000
      FF000000FF000000FF0000000000000000000000000000000000000000000000
      0000000000000000FF000000FF000000FF000000FF000000FF00000000000000
      0000000000000000000000000000000000000000FF000000FF00000000000000
      00000000000000000000000000000000FF000000FF000000FF00000000000000
      0000000000000000FF000000FF00000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000FF000000FF000000FF000000
      FF00000000000000000000000000000000000000000000000000000000000000
      0000000000000000FF000000FF000000FF000000FF000000FF00000000000000
      0000000000000000000000000000000000000000FF000000FF00000000000000
      0000000000000000000000000000000000000000FF000000FF000000FF000000
      0000000000000000FF000000FF00000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000FF000000FF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000FF000000FF000000FF0000000000000000000000
      000000000000000000000000000000000000000000000000FF000000FF000000
      000000000000000000000000000000000000000000000000FF000000FF000000
      FF00000000000000FF0000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000FF000000FF000000FF0000000000000000000000
      000000000000000000000000000000000000000000000000FF000000FF000000
      FF000000000000000000000000000000000000000000000000000000FF000000
      FF000000FF000000FF0000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      00000000000000000000000000000000000000000000000000000000FF000000
      FF000000FF000000000000000000000000000000000000000000000000000000
      FF000000FF000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      FF000000FF000000FF000000FF000000FF000000FF000000FF000000FF000000
      FF00000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000FF000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000FF000000FF000000FF000000FF000000FF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000424D3E000000000000003E000000
      2800000040000000300000000100010000000000800100000000000000000000
      000000000000000000000000FFFFFF00FFFF000000000000F83F000000000000
      E7CF000000000000DFF7000000000000B01B000000000000B83B000000000000
      7C7D0000000000007E8D00000000000041050000000000004105000000000000
      4105000000000000818B00000000000081FB000000000000DFF7000000000000
      E7CF000000000000F83F000000000000FFFFFFFDFFFD8001FFFFFFF8FFF80000
      FFFFFFF1FFF10000FF3FFFE3FFE30000FC3FFFC7FFC70000F03FE08FE08F0000
      C000C01FC01F00000000803F803F0000C000001F001FE007F03F001F001FE007
      FC3F001F001FE007FF3F001F001FE007FFFF001F001FE00FFFFF803F803FE01F
      FFFFC07FC07FE03FFFFFE0FFE0FFE07FFFFFFEFFFFFFFC7FF83FFEFFFFFFFC7F
      E00FFC7FFFFFFC7FCFC7FC7FFFFFFC7F87E3F83FFCFFFC7FA3F3F83FFC3FFC7F
      31F9F01FFC0FE00F38F9F01F0003E00F3C79E00F0000F01F3E39E00F0003F01F
      3F19FC7FFC0FF83F9F8BFC7FFC3FF83F8FC3FC7FFCFFFC7FC7E7FC7FFFFFFC7F
      E00FFC7FFFFFFEFFF83FFC7FFFFFFEFF00000000000000000000000000000000
      000000000000}
  end
  object MainMenu1: TMainMenu
    Images = ImageList1
    Left = 304
    Top = 128
    object file1: TMenuItem
      Caption = 'File'
      object Open1: TMenuItem
        Caption = 'Open'
        OnClick = Button_Filename1Click
      end
      object Addtrace1: TMenuItem
        Caption = 'Add trace'
        OnClick = Button_add_trace1Click
      end
      object Clearalltraces1: TMenuItem
        Caption = 'Clear all traces'
        OnClick = Button_clear_all_traces1Click
      end
      object Save1: TMenuItem
        Caption = 'Save'
        object SavePlotAs1: TMenuItem
          Caption = 'Save Plot As'
          OnClick = ButtonSave1Click
        end
        object PlottoClipboard1: TMenuItem
          Caption = 'Plot to Clipboard'
          OnClick = Button_PlotToClipboard1Click
        end
        object SaveDataAs1: TMenuItem
          Caption = 'Save Data As CSV'
          OnClick = SaveDataAs1Click
        end
      end
      object Exit1: TMenuItem
        Caption = 'Exit'
        OnClick = Exit1Click
      end
    end
    object Scales1: TMenuItem
      Caption = 'Scales'
      OnClick = Scales1Click
    end
    object Help1: TMenuItem
      Caption = 'Help'
      object Action11: TMenuItem
        Action = Action1
        Caption = 'Manual'
      end
      object About1: TMenuItem
        Caption = 'About'
        OnClick = About1Click
      end
    end
    object Panel3: TMenuItem
      Caption = 'Panel'
      Enabled = False
      Visible = False
      object visible1: TMenuItem
        Caption = 'visible'
        Enabled = False
        Visible = False
        OnClick = visible1Click
      end
    end
  end
  object Opencsv: TOpenDialog
    Ctl3D = False
    DefaultExt = 'csv'
    Filter = 'csv files|*.csv|all files|*.*'
    OptionsEx = [ofExNoPlacesBar]
    Title = 'Open CSV file'
    Left = 384
    Top = 88
  end
  object SavePictureDialog1: TSavePictureDialog
    DefaultExt = 'bmp'
    Filter = 'Bitmaps (*.bmp)|*.bmp'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Left = 192
    Top = 200
  end
  object Opencsv1: TOpenDialog
    DefaultExt = 'csv'
    Filter = 'csv files|*.csv|all files|*.*'
    Left = 400
    Top = 192
  end
  object FileOpenDialogCSV: TFileOpenDialog
    DefaultExtension = 'csv'
    FavoriteLinks = <>
    FileTypes = <
      item
        DisplayName = 'csv'
        FileMask = '*.csv'
      end
      item
        DisplayName = 'all'
        FileMask = '*.*'
      end>
    Options = []
    Title = 'Open CSV file'
    Left = 512
    Top = 176
  end
  object FileSaveDialog1: TFileSaveDialog
    DefaultExtension = 'bmp'
    FavoriteLinks = <>
    FileTypes = <
      item
        DisplayName = 'all'
        FileMask = '*.*'
      end
      item
        DisplayName = 'bmp'
        FileMask = '*.bmp'
      end
      item
        DisplayName = 'jpg'
        FileMask = '*.jpg'
      end
      item
        DisplayName = 'jpeg'
        FileMask = '*.jpeg'
      end
      item
        DisplayName = 'gif'
        FileMask = '*.gif'
      end
      item
        DisplayName = 'png'
        FileMask = '*.png'
      end>
    Options = [fdoOverWritePrompt, fdoPathMustExist]
    Title = 'Save Plot as an Image'
    Left = 152
    Top = 304
  end
  object FileSaveDialogCSV: TFileSaveDialog
    DefaultExtension = 'csv'
    FavoriteLinks = <>
    FileTypes = <
      item
        DisplayName = 'csv'
        FileMask = '*.csv'
      end
      item
        DisplayName = 'all files'
        FileMask = '*.*'
      end>
    Options = [fdoOverWritePrompt, fdoPathMustExist]
    Title = 'Save CSV file as'
    Left = 232
    Top = 304
  end
  object ColourDialog1: TColorDialog
    CustomColors.Strings = (
      'ColorA=A65A08'
      'ColorB=29BD6E'
      'ColorC=595A5A'
      'ColorD=FF00FF'
      'ColorE=0000FF'
      'ColorF=00E0FF'
      'ColorG=007FFF'
      'ColorH=FF0000'
      'ColorI=00FF00'
      'ColorJ=000000')
    Options = [cdFullOpen]
    Left = 624
    Top = 272
  end
end
