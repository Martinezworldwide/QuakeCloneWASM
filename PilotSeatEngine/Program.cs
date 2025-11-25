using Microsoft.AspNetCore.Components.Web;
using Microsoft.AspNetCore.Components.WebAssembly.Hosting;
using PilotSeatEngine;

var builder = WebAssemblyHostBuilder.CreateDefault(args);
// Blazor app will be mounted in #pilot-seat-app element
// Only add RootComponents once (prevent duplicate HeadOutlet registration)
builder.RootComponents.Add<App>("#pilot-seat-app");
// Note: HeadOutlet is added by default in CreateDefault, but we need to ensure it's only added once
// If HeadOutlet causes issues, we can remove it since we're using NoLayout
// builder.RootComponents.Add<HeadOutlet>("head::after");

// Configure base address for subdirectory deployment
builder.Services.AddScoped(sp => new HttpClient { 
    BaseAddress = new Uri(builder.HostEnvironment.BaseAddress) 
});

await builder.Build().RunAsync();
