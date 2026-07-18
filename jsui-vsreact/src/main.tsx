import { render, configureTheme, Text, VERSION } from "@vsreact/core";
import {
  posthog,
  useEditorSession,
  useScreen,
  PostHogErrorBoundary,
} from "@vsreact/posthog";
import App from "./App";

configureTheme({
  colors: {
    background: "#060806",
    panel: "#0D100C",
    lift: "#12160F",
    well: "#090B08",
    line: "#242B20",
    lineSoft: "#1A1F17",
    accent: "#C6F135",
    text: "#EDF1E4",
    muted: "#848D7B",
    faint: "#5A6253",
    error: "#FF5D6C",
  },
});

// Product analytics + error tracking (the API key lives in C++; the
// native PostHogBridge delivers batches off-thread).
posthog.init({
  defaultProperties: { plugin: "stashtrack", sdk_version: VERSION },
});

function AppWithAnalytics() {
  useEditorSession();
  useScreen("Main");
  return <App />;
}

render(
  <PostHogErrorBoundary
    fallback={<Text className="text-text p-6">StashTrack's UI crashed — reopen the window.</Text>}
  >
    <AppWithAnalytics />
  </PostHogErrorBoundary>,
);
