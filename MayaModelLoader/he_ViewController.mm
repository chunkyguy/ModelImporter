//
//  he_ViewController.m
//  MayaModelLoader
//
//  Created by Sid on 09/12/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//

#import "he_ViewController.h"
//#import "he_FBXImporter.h"
//#import "he_OBJImporter.h"
//#import "he_Assimp.h"
#import "he_Scene.h"



@interface he_ViewController () {
}
@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;
@end

@implementation he_ViewController

- (void)viewDidLoad
{
 [super viewDidLoad];


 self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
 
 if (!self.context) {
  NSLog(@"Failed to create ES context");
 }
 
 GLKView *view = (GLKView *)self.view;
 view.context = self.context;
 view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
 
 [self setupGL];
}

- (void)dealloc
{
 [self tearDownGL];
 
 if ([EAGLContext currentContext] == self.context) {
  [EAGLContext setCurrentContext:nil];
 }
}

- (void)didReceiveMemoryWarning
{
 [super didReceiveMemoryWarning];
 
 if ([self isViewLoaded] && ([[self view] window] == nil)) {
  self.view = nil;
  
  [self tearDownGL];
  
  if ([EAGLContext currentContext] == self.context) {
   [EAGLContext setCurrentContext:nil];
  }
  self.context = nil;
 }
 
 // Dispose of any resources that can be recreated.
}

- (void)setupGL
{
 [EAGLContext setCurrentContext:self.context];

 const char *model_file = [[[NSBundle mainBundle] pathForResource:@"cube" ofType:@"dae"] UTF8String];

 const char *txvsh_src = [[NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"txShader" ofType:@"vsh"] encoding:NSUTF8StringEncoding error:nil] UTF8String];
 const char *txfsh_src = [[NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"txShader" ofType:@"fsh"] encoding:NSUTF8StringEncoding error:nil] UTF8String];

 const char *clrvsh_src = [[NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"clrShader" ofType:@"vsh"] encoding:NSUTF8StringEncoding error:nil] UTF8String];
 const char *clrfsh_src = [[NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"clrShader" ofType:@"fsh"] encoding:NSUTF8StringEncoding error:nil] UTF8String];

 he_Scene::Create(model_file,
                  txvsh_src, txfsh_src,
                  clrvsh_src, clrfsh_src);
}

- (void)tearDownGL
{
 [EAGLContext setCurrentContext:self.context];
 he_Scene::Destroy();
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
 he_Scene::Reshape(self.view.bounds.size.width, self.view.bounds.size.height);
 he_Scene::Update(self.timeSinceLastUpdate * 1000);;
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
 he_Scene::Render();
}
@end
