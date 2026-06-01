import { API_BASE_URL } from '../config/env';
import type {
  AnalyticsResponse,
  AutocompleteResponse,
  HealthResponse,
  SearchResponse,
} from '../types/api';

async function requestJson<T>(path: string, init?: RequestInit): Promise<T> {
  const response = await fetch(`${API_BASE_URL}${path}`, {
    ...init,
    headers: {
      'Content-Type': 'application/json',
      ...(init?.headers ?? {}),
    },
  });

  if (!response.ok) {
    let message = `Request failed with status ${response.status}`;
    try {
      const body = (await response.json()) as { detail?: string };
      message = body.detail ?? message;
    } catch {
      // Keep the status-based message when the backend returns non-JSON.
    }
    throw new Error(message);
  }

  return (await response.json()) as T;
}

export const api = {
  search(query: string, topK: number, explain: boolean): Promise<SearchResponse> {
    return requestJson<SearchResponse>('/search', {
      method: 'POST',
      body: JSON.stringify({ query, top_k: topK, explain }),
    });
  },

  autocomplete(query: string, limit = 8): Promise<AutocompleteResponse> {
    const params = new URLSearchParams({ q: query, limit: String(limit) });
    return requestJson<AutocompleteResponse>(`/autocomplete?${params.toString()}`);
  },

  analytics(limit = 10): Promise<AnalyticsResponse> {
    const params = new URLSearchParams({ limit: String(limit) });
    return requestJson<AnalyticsResponse>(`/analytics?${params.toString()}`);
  },

  health(): Promise<HealthResponse> {
    return requestJson<HealthResponse>('/health');
  },
};
